// -*- C++ -*-
////////////////////////////////////////////////////////////////////////////////
///
/// \file   utils/src/radix.cc
///
/// \author Walter Dehnen
/// \date   2011,2012
///
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2011,2012 Walter Dehnen
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but          
// WITHOUT ANY WARRANTY; without even the implied warranty of                   
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU            
// General Public License for more details.                                     
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc., 675
// Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
#include <radix.h>
#include <sse.h>

#ifdef __INTEL_COMPILER
#pragma warning (disable:981)  /* operands are evaluated in unspecified order */
#endif

//
#ifdef WDutilsDevel
namespace WDutils {
#ifdef _OPENMP
  namespace Radix {
    /// global datum: pointers to counters across all threads
    /// \note these are externally linkable and used in devel/radixP.h
    int* B[OMP::MaxNumThreadsInFunc];
  }
#else
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wpedantic"
# warning openMP not supported: implementing serial version for Radix::PSort()
# pragma clang diagnostic pop
#endif
}
//
namespace {
  using namespace WDutils;
#ifdef _OPENMP
  using WDutils::Radix::B;
#endif
  /// parallel radix sort for float or double
  template<typename X> class ParallelRadixSort {
#ifdef _OPENMP
    /// type used to map X to and from
    typedef typename Radix::SortTraits<X>::integer_type integer;
    /// mask and counter size in sections 1,2,4,5
    static const int n0=0x800,m0=0x7ff;
    /// mask and counter size in sections 3,6
    static const int n2=0x400,m2=0x3ff;
    /// \name local data
    //@{
    const int      It,Nt;    ///< rank of local thread; # threads
    const unsigned i0,iN;    ///< begin and end of local data partition
    int     *const L;        ///< local counters
    integer *const iX;       ///< X array seen as unsigned int
    integer *const iY;       ///< Y array seen as unsigned int
    //@}
    /// auxiliary for sort(): first-section count (converts)
    void first_count(integer*to)
    {
      for(int i=i0; i!=n0; ++i)
	L[i] = 0;
      for(unsigned i=i0; i!=iN; ++i) {
	integer f=iX[i];
	to[i]= f^= Radix::SortTraits<X>::forward_map(f);
	L[f&m0]++;
      }
    }
    /// auxiliary for sort(): ordinary count (any but first section)
    template<int shift, int mask> void count(integer*in)
    {
      for(unsigned i=i0; i<=mask; ++i)
	L[i] = 0;
      for(unsigned i=i0; i!=iN; ++i)
	L[(in[i]>>shift) & mask]++;
    }
    /// auxialiary for sort(): cumulates counts
    void cumulate(const int n)
    { Radix::cumulate(n,It,Nt); }
    /// auxialiary for sort(): slots data back in
    template<int shift, int mask> void slot(const integer*in, integer*to)
    {
      for(unsigned i=i0; i!=iN; ++i) 
	to[L[(in[i]>>shift)&mask]++]=in[i];
    }
    /// auxialiary for sort(): slots data back in and converts back to X
    template<int shift> void last_slot()
    {
      for(unsigned i=i0; i!=iN; ++i) {
	integer f=iY[i];
	iX[L[(f>>shift)&m2]++] = f^ Radix::SortTraits<X>::backward_map(f);
      }
    }
    /// dtor: delete memory for counter
    ~ParallelRadixSort()
    { WDutils_DEL16(L); }
    /// ctor: allocate counter, take data arrays, share counter, set partition
    /// \param[in]      n  size of arrays
    /// \param[in,out]  x  array to sort/sorted
    /// \param[in,out]  y  array to sort/sorted
    ParallelRadixSort(unsigned n, X*x, X*y)
      : It(OMP::Rank()), Nt(OMP::TeamSize()), i0(0), iN(0),
	L (WDutils_NEW16(int,n0)),
	iX(reinterpret_cast<integer*>(x)),
	iY(reinterpret_cast<integer*>(y))
    {
      // share local counters
      B[It]=L;
      // create partition of data
      OMP::Partition(n,It,Nt,
		     const_cast<unsigned&>(i0),const_cast<unsigned&>(iN));
    }
    /// perform radix sort in parallel
    void sort();
#endif // _OPENMP
  public:
    /// wrapper for Radix::PSort(n,x,y,w)
    static void sortP(unsigned n, X*x, X*y, bool
#ifdef _OPENMP
		      warn
#endif
		      )
    {
#ifdef _OPENMP
      if(OMP::IsParallel() && OMP::TeamSize()>1) {
	if(warn && OMP::Rank()==0)
	  WDutils_WarningF("called from within OMP parallel region: "
			   "we assume shared global data "
			   "(use WDutils::Radix::Sort() for sorting local "
			   "data).\n");
#pragma omp barrier
#ifndef __INTEL_COMPILER
        try {
#endif
	  ParallelRadixSort<X> P(n,x,y);
	  P.sort();
#ifndef __INTEL_COMPILER
	} catch(WDutils::exception E)  { WDutils_ErrorN(text(E)); }
#endif
      } else if(!OMP::IsParallel() && OMP::MaxNumThreads()>1) {
	ThrowGuard Guard;
#pragma omp parallel
	{
	  ParallelRadixSort<X> P(n,x,y);
	  P.sort();
	}
      } else
#endif
	Radix::Sort(n,x,y);
    }
    /// wrapper for Radix::PSort(n,x,w)
    static void sortP(unsigned n, X*x, bool
#ifdef _OPENMP
		      warn
#endif
		      )
    {
      X*y;
#ifdef _OPENMP
      if(OMP::IsParallel() && OMP::TeamSize()>1) {
	if(warn && OMP::Rank()==0)
	  WDutils_WarningF("called from within OMP parallel region: "
			   "we assume shared global data "
			   "(use WDutils::Radix::Sort() for sorting local "
			   "data).\n");
# ifndef __INTEL_COMPILER
	try {
# endif
	  if(OMP::Rank() == 0) y=WDutils_NEW16(X,n);
#pragma omp barrier
	  ParallelRadixSort<X> P(n,x,y);
	  P.sort();
	  if(OMP::Rank() == 0) WDutils_DEL16(y);
# ifndef __INTEL_COMPILER
	} catch(WDutils::exception E)  { WDutils_ErrorN(text(E)); }
# endif
      } else
#endif
      {
	y=WDutils_NEW16(X,n);
#ifdef _OPENMP
	if(!OMP::IsParallel() && OMP::MaxNumThreads()>1)
#pragma omp parallel
	  {
# ifndef __INTEL_COMPILER
	    try {
# endif
	      ParallelRadixSort<X> P(n,x,y);
	      P.sort();
# ifndef __INTEL_COMPILER
	    } catch(WDutils::exception E) { WDutils_ErrorN(text(E)); }
# endif
	  }
	else 
#endif
	  Radix::Sort(n,x,y);
	WDutils_DEL16(y);
      }
    }
  };
#ifdef _OPENMP
  //
  template<> void ParallelRadixSort<float>::sort()
  {
    // 1 convert iX to integer, copy to iY, sorted in iX
    first_count(iY);
    cumulate(n0);
    slot<0,m0>(iY,iX);
#pragma omp barrier
    // 2 input in iX, sorted into iY, no conversion
    count<11,m0>(iX);
    cumulate(n0);
    slot<11,m0>(iX,iY);
#pragma omp barrier
    // 3 input in iY, sorted into iX, convert back to float
    count<22,m2>(iY);
    cumulate(n2);
    last_slot<22>();
#pragma omp barrier
  }
  //
  template<> void ParallelRadixSort<double>::sort()
  {
    // 1 convert iX to integer, sorted in iY
    first_count(iX);
    cumulate(n0);
    slot<0,m0>(iX,iY);
#pragma omp barrier
    // 2 input in iY, sorted into iX, no conversion
    count<11,m0>(iY);
    cumulate(n0);
    slot<11,m0>(iY,iX);
#pragma omp barrier
    // 3 input in iX, sorted into iY, no conversion
    count<22,m2>(iX);
    cumulate(n0);
    slot<22,m2>(iX,iY);
#pragma omp barrier
    // 4 input in iY, sorted into iX, no conversion
    count<32,m0>(iY);
    cumulate(n0);
    slot<32,m0>(iY,iX);
#pragma omp barrier
    // 5 input in iX, sorted into iY, no conversion
    count<43,m0>(iX);
    cumulate(n0);
    slot<43,m0>(iX,iY);
#pragma omp barrier
    // 6 input in iY, sorted into iX, convert back to double
    count<54,m2>(iY);
    cumulate(n2);
    last_slot<54>();
  }
#endif // _OPENMP
  //
} // namespace {
//
void WDutils::Radix::PSort(unsigned n, float*x, float*y, bool w)
{ ::ParallelRadixSort<float>::sortP(n,x,y,w); }
//
void WDutils::Radix::PSort(unsigned n, float*x, bool w)
{ ::ParallelRadixSort<float>::sortP(n,x,w); }
//
void WDutils::Radix::PSort(unsigned n, double*x, double*y, bool w)
{ ::ParallelRadixSort<double>::sortP(n,x,y,w); }
//
void WDutils::Radix::PSort(unsigned n, double*x, bool w)
{ ::ParallelRadixSort<double>::sortP(n,x,w); }
#endif // WDutilsDevel
//
void WDutils::Radix::Sort(unsigned N, float*X, float*Y)
{
  static const int n0=0x800,n2=0x400;
  static const int m0=0x7ff,m2=0x3ff;
  int B0[n0+n0+n2]={0},*B1=B0+n0,*B2=B1+n0;
  uint32_t*iX=reinterpret_cast<uint32_t*>(X);
  uint32_t*iY=reinterpret_cast<uint32_t*>(Y);
  for(uint32_t i=0; i!=N; ++i) {
    register uint32_t f=iX[i];
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wsign-conversion"
    iY[i]=f^=-int32_t(f>>31)|0x80000000;
# pragma clang diagnostic pop
    B0[f       &m0]++;
    B1[(f>>=11)&m0]++;
    B2[(f>>=11)&m2]++;
  }
  for(int i=0,s=0,t; i!=n0; ++i) { t=B0[i]; B0[i]=s; s+=t; }
  for(int i=0,s=0,t; i!=n0; ++i) { t=B1[i]; B1[i]=s; s+=t; }
  for(int i=0,s=0,t; i!=n2; ++i) { t=B2[i]; B2[i]=s; s+=t; }
  for(uint32_t i=0; i!=N; ++i) iX[B0[iY[i]     &m0]++] = iY[i];
  for(uint32_t i=0; i!=N; ++i) iY[B1[iX[i]>>11 &m0]++] = iX[i];
  for(uint32_t i=0; i!=N; ++i) {
    register uint32_t f=iY[i];
    iX[B2[f>>22 &m2]++]=f^(((f>>31)-1)|0x80000000); 
  }
}
//
void WDutils::Radix::Sort(unsigned N, double*X, double*Y)
{
  static const uint32_t n0=0x800,n2=0x400, n=n0+n0+n2;
  static const uint64_t m0=0x7ff,m2=0x3ff;
  uint32_t B0[n]={0},*B1=B0+n0,*B2=B1+n0;
  uint64_t*iX=reinterpret_cast<uint64_t*>(X);
  uint64_t*iY=reinterpret_cast<uint64_t*>(Y);
  // sort the lower 32 bits
  for(uint32_t i=0; i!=N; ++i) {
    register uint64_t f=iX[i];
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wsign-conversion"
    iX[i]=f^=-int64_t(f>>63) | 0x8000000000000000ll;
# pragma clang diagnostic pop
    B0[f       &m0] ++;
    B1[(f>>=11)&m0] ++;
    B2[(f>>=11)&m2] ++;
  }
  for(uint32_t i=0,s=0,t; i!=n0; ++i) { t=B0[i]; B0[i]=s; s+=t; }
  for(uint32_t i=0,s=0,t; i!=n0; ++i) { t=B1[i]; B1[i]=s; s+=t; }
  for(uint32_t i=0,s=0,t; i!=n2; ++i) { t=B2[i]; B2[i]=s; s+=t; }
  for(uint32_t i=0; i!=N; ++i) iY[B0[iX[i]     &m0]++] = iX[i];
  for(uint32_t i=0; i!=N; ++i) iX[B1[iY[i]>>11 &m0]++] = iY[i];
  for(uint32_t i=0; i!=N; ++i) iY[B2[iX[i]>>22 &m2]++] = iX[i];
  // sort the upper 32 bits
  for(uint32_t i=0; i!=n; ++i) B0[i]=0;
  for(uint32_t i=0; i!=N; ++i) {
    register uint64_t f=iY[i];
    B0[(f>>=32)&m0] ++;
    B1[(f>>=11)&m0] ++;
    B2[(f>>=11)&m2] ++;
  }
  for(uint32_t i=0,s=0,t; i!=n0; ++i) { t=B0[i]; B0[i]=s; s+=t; }
  for(uint32_t i=0,s=0,t; i!=n0; ++i) { t=B1[i]; B1[i]=s; s+=t; }
  for(uint32_t i=0,s=0,t; i!=n2; ++i) { t=B2[i]; B2[i]=s; s+=t; }
  for(uint32_t i=0; i!=N; ++i) iX[B0[iY[i]>>32 &m0]++] = iY[i];
  for(uint32_t i=0; i!=N; ++i) iY[B1[iX[i]>>43 &m0]++] = iX[i];
  for(uint32_t i=0; i!=N; ++i) {
    register uint64_t f = iY[i];
    iX[B2[f    >>54 &m2]++] =  f ^ (((f>>63) - 1) | 0x8000000000000000ll);
  }
}
//
