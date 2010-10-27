// -*-Mode: C++;-*-

// * BeginRiceCopyright *****************************************************
//
// $HeadURL$
// $Id$
//
// --------------------------------------------------------------------------
// Part of HPCToolkit (hpctoolkit.org)
// 
// Information about sources of support for research and development of
// HPCToolkit is at 'hpctoolkit.org' and in 'README.Acknowledgments'.
// --------------------------------------------------------------------------
// 
// Copyright ((c)) 2002-2010, Rice University 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
// 
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage. 
// 
// ******************************************************* EndRiceCopyright *

//***************************************************************************
//
// class Prof::Metric::AExpr and derived classes
//
// An abstract expression that can represents derived expressions that
// are directly computed in the sense that all inputs are known at the
// same time (cf. incrementally computed metrics where this is not true).
//
// Assumes all sources are known when evaluation rule is invoked.
//
// Currently supported expressions are
//   Const  : double constant                      : leaf
//   Var    : variable with a String name          : leaf
//   Neg    : negation                             : unary
//   Power  : power expressed in base and exponent : binary
//   Divide : division expression                  : binary
//   Minus  : subtraction expression               : binary
//   Plus   : addition expression                  : n-ary
//   Times  : multiplication expression            : n-ary
//   Min    : min expression                       : n-ary
//   Max    : max expression                       : n-ary
//   Mean   : mean (arithmetic) expression         : n-ary
//   StdDev : standard deviation expression        : n-ary
//   CoefVar: coefficient of variance              : n-ary
//   RStdDev: relative standard deviation          : n-ary
//
//***************************************************************************

#ifndef prof_Prof_Metric_AExpr_hpp
#define prof_Prof_Metric_AExpr_hpp

//************************ System Include Files ******************************

#include <iostream> 
#include <string>
#include <algorithm>

//************************* User Include Files *******************************

#include "Metric-IData.hpp"
#include "Metric-IDBExpr.hpp"

#include <lib/support/NaN.h>
#include <lib/support/Unique.hpp>
#include <lib/support/StrUtil.hpp>


//************************ Forward Declarations ******************************

//****************************************************************************

namespace Prof {

namespace Metric {

// ----------------------------------------------------------------------
// class AExpr
//   The base class for all concrete evaluation classes
// ----------------------------------------------------------------------

class AExpr
  : public IDBExpr,
    public Unique // disable copying, for now
{
  // TODO: replace AExpr** with AExprVec
public:
  typedef std::vector<AExpr*> AExprVec;

  static const double epsilon = 0.000001;
  
public:
  AExpr()
    : m_accumId(Metric::IData::npos), m_accum2Id(Metric::IData::npos),
      m_numSrcVarId(Metric::IData::npos)
  { }

  virtual ~AExpr()
  { }

  // ------------------------------------------------------------
  //
  // ------------------------------------------------------------

  // eval: generate a finalized value and return result
  virtual double
  eval(const Metric::IData& mdata) const = 0;

  // evalNF: generate a non-finalized values and store results in accumulators
  virtual double
  evalNF(Metric::IData& mdata) const
  {
    double z = eval(mdata);
    accumVar(mdata) = z;
    return z;
  }


  static bool
  isok(double x)
  { return !(c_isnan_d(x) || c_isinf_d(x)); }


  // ------------------------------------------------------------
  // Metric::IDBExpr: exported formulas for Flat and Callers view
  // ------------------------------------------------------------

  virtual std::string
  combineString1() const
  { DIAG_Die(DIAG_Unimplemented); return ""; }

  virtual std::string
  combineString2() const
  { DIAG_Die(DIAG_Unimplemented); return ""; }

  virtual std::string
  finalizeString() const
  { DIAG_Die(DIAG_Unimplemented); return ""; }


  // ------------------------------------------------------------
  // Metric::IDBExpr: primitives
  // ------------------------------------------------------------

  virtual uint
  accumId() const
  { return m_accumId; }

  void
  accumId(uint x)
  { m_accumId = x; }


  // ------------------------------------------------------------
  // Metric::IDBExpr: primitives
  // ------------------------------------------------------------

  bool
  isSetAccum2() const
  { return (m_accum2Id != Metric::IData::npos); }

  virtual bool
  hasAccum2() const
  { return false; }

  virtual uint
  accum2Id() const
  { return m_accum2Id; }

  void
  accum2Id(uint x)
  { m_accum2Id = x; }


  // ------------------------------------------------------------
  // Metric::IDBExpr: primitives
  // ------------------------------------------------------------

  virtual bool
  hasNumSrcVar() const
  { return false; }


  virtual uint
  numSrcFxd() const
  { DIAG_Die(DIAG_Unimplemented); return 0; }


  virtual uint
  numSrcVarId() const
  { return m_numSrcVarId; }

  void
  numSrcVarId(uint x)
  { m_numSrcVarId = x; }


  // ------------------------------------------------------------
  //
  // ------------------------------------------------------------

  static double&
  var(Metric::IData& mdata, uint mId)
  { return mdata.demandMetric(mId); }

  double&
  accumVar(Metric::IData& mdata) const
  { return var(mdata, m_accumId); }

  double&
  accum2Var(Metric::IData& mdata) const
  { return var(mdata, m_accum2Id); }


  // ------------------------------------------------------------
  //
  // ------------------------------------------------------------

  virtual std::string
  toString() const;


  virtual std::ostream&
  dump(std::ostream& os = std::cout) const
  {
    dumpMe(os);
    return os;
  }

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const = 0;

  // Metric::IDBExpr::ddump()

protected:

  // ------------------------------------------------------------
  //
  // ------------------------------------------------------------

  static double
  evalSum(const Metric::IData& mdata, AExpr** opands, int sz)
  {
    double result = 0.0;
    for (int i = 0; i < sz; ++i) {
      double x = opands[i]->eval(mdata);
      result += x;
    }
    return result;
  }


  static std::pair<double, double>
  evalSumSquares(const Metric::IData& mdata, AExpr** opands, int sz)
  {
    double result1 = 0.0; // sum
    double result2 = 0.0; // sum of squares
    for (int i = 0; i < sz; ++i) {
      double x = opands[i]->eval(mdata);
      result1 += x;
      result2 += (x * x);
    }
    return std::make_pair(result1, result2);
  }


  static double
  evalMean(const Metric::IData& mdata, AExpr** opands, int sz)
  {
    double sum = evalSum(mdata, opands, sz);
    double result = sum / (double) sz;
    return result;
  }

  
  // returns <variance, mean>
  static std::pair<double, double>
  evalVariance(const Metric::IData& mdata, AExpr** opands, int sz)
  {
    double* x = new double[sz];
    
    double x_mean = 0.0; // mean
    for (int i = 0; i < sz; ++i) {
      double t = opands[i]->eval(mdata);
      x[i] = t;
      x_mean += t;
    }
    x_mean = x_mean / sz;
    
    double x_var = 0.0; // variance
    for (int i = 0; i < sz; ++i) {
      double t = (x[i] - x_mean);
      t = t * t;
      x_var += t;
    }
    x_var = x_var / sz;
    delete[] x;
    
    return std::make_pair(x_var, x_mean);
  }


  double
  evalStdDevNF(Metric::IData& mdata, AExpr** opands, int sz) const
  {
    std::pair<double, double> z = evalSumSquares(mdata, opands, sz);
    double z1 = z.first;  // sum
    double z2 = z.second; // sum of squares
    accumVar(mdata) = z1;
    accum2Var(mdata) = z2;
    return z1;
  }


  static void 
  dump_opands(std::ostream& os, AExpr** opands, int sz, const char* sep = ", ");
  
protected:
  uint m_accumId;     // used only for Metric::IDBExpr routines
  uint m_accum2Id;    // used only for Metric::IDBExpr routines
  uint m_numSrcVarId; // used only for Metric::IDBExpr routines
};


// ----------------------------------------------------------------------
// class Const
//   Represent a double constant
// ----------------------------------------------------------------------

class Const
  : public AExpr
{
public:
  Const(double c) 
    : m_c(c)
  { }

  ~Const()
  { }

  virtual double
  eval(const Metric::IData& mdata) const
  { return m_c; }


  // ------------------------------------------------------------
  // Metric::IDBExpr: exported formulas for Flat and Callers view
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return 1; }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;
  
private:
  double m_c;
};


// ----------------------------------------------------------------------
// class Neg
//   Represent a negative value of an AExpr
// ----------------------------------------------------------------------

class Neg
  : public AExpr
{  
public:
  // Assumes ownership of AExpr
  Neg(AExpr* expr)
  { m_expr = expr; }

  ~Neg()
  { delete m_expr; }

  virtual double
  eval(const Metric::IData& mdata) const;


  // ------------------------------------------------------------
  // Metric::IDBExpr: exported formulas for Flat and Callers view
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return 1; }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;
  
private:
  AExpr* m_expr;
};


// ----------------------------------------------------------------------
// class Var
//   Represent a variable
// ----------------------------------------------------------------------

class Var
  : public AExpr
{
public:
  Var(std::string name, int metricId)
    : m_name(name), m_metricId(metricId) 
  { }
  
  ~Var()
  { }

  virtual double
  eval(const Metric::IData& mdata) const
  { return mdata.demandMetric(m_metricId); }

  
  // ------------------------------------------------------------
  // Metric::IDBExpr: exported formulas for Flat and Callers view
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return 1; }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;
  
private:
  std::string m_name;
  int m_metricId;
};


// ----------------------------------------------------------------------
// class Power
//   Represent a power expression
// ----------------------------------------------------------------------

class Power
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  Power(AExpr* base, AExpr* exponent)
    : m_base(base), m_exponent(exponent) 
  { }

  ~Power()
  { 
    delete m_base; 
    delete m_exponent; 
  }

  virtual double
  eval(const Metric::IData& mdata) const;


  // ------------------------------------------------------------
  // Metric::IDBExpr:
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return 2; }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr* m_base;
  AExpr* m_exponent;
};


// ----------------------------------------------------------------------
// class Divide
//   Represent the division
// ----------------------------------------------------------------------

class Divide
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  Divide(AExpr* numerator, AExpr* denominator)
    : m_numerator(numerator), m_denominator(denominator) 
  { }

  ~Divide()
  { 
    delete m_numerator; 
    delete m_denominator; 
  }


  virtual double
  eval(const Metric::IData& mdata) const;

  // ------------------------------------------------------------
  // Metric::IDBExpr:
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return 2; }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr* m_numerator;
  AExpr* m_denominator;
};


// ----------------------------------------------------------------------
// class Minus
//   Represent the subtraction
// ----------------------------------------------------------------------

class Minus
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  Minus(AExpr* minuend, AExpr* subtrahend)
    : m_minuend(minuend), m_subtrahend(subtrahend) 
  { }

  ~Minus()
  {
    delete m_minuend;
    delete m_subtrahend;
  }

  virtual double
  eval(const Metric::IData& mdata) const;

  // ------------------------------------------------------------
  // Metric::IDBExpr:
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return 2; }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr* m_minuend;
  AExpr* m_subtrahend;
};


// ----------------------------------------------------------------------
// class Plus
//   Represent addition
// ----------------------------------------------------------------------

class Plus
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  Plus(AExpr** oprnds, int numOprnds)
    : m_opands(oprnds), m_sz(numOprnds) 
  { }

  ~Plus();
  
  virtual double
  eval(const Metric::IData& mdata) const;

  // ------------------------------------------------------------
  // Metric::IDBExpr:
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return m_sz; }


  virtual std::string
  combineString1() const
  { return combineString1Sum(); }

  virtual std::string
  finalizeString() const
  { return finalizeStringSum(); }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr** m_opands;
  int m_sz;
};


// ----------------------------------------------------------------------
// class Times
//   Represent multiplication
// ----------------------------------------------------------------------

class Times
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  Times(AExpr** oprnds, int numOprnds)
    : m_opands(oprnds), m_sz(numOprnds) 
  { }

  ~Times();

  virtual double
  eval(const Metric::IData& mdata) const;

  // ------------------------------------------------------------
  // Metric::IDBExpr:
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return m_sz; }


  // TODO: combineString1(), finalizeString()


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr** m_opands;
  int m_sz;
};


// ----------------------------------------------------------------------
// Min
// ----------------------------------------------------------------------

class Min
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  Min(AExpr** oprnds, int numOprnds)
    : m_opands(oprnds), m_sz(numOprnds) 
  { }

  ~Min();

  virtual double
  eval(const Metric::IData& mdata) const;

  // ------------------------------------------------------------
  // Metric::IDBExpr:
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return m_sz; }


  virtual std::string
  combineString1() const
  { return combineString1Min(); }

  virtual std::string
  finalizeString() const
  { return finalizeStringMin(); }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr** m_opands;
  int m_sz;
};


// ----------------------------------------------------------------------
// Max
// ----------------------------------------------------------------------

class Max
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  Max(AExpr** oprnds, int numOprnds)
    : m_opands(oprnds), m_sz(numOprnds) 
  { }

  ~Max();

  virtual double
  eval(const Metric::IData& mdata) const;

  // ------------------------------------------------------------
  // Metric::IDBExpr:
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return m_sz; }


  virtual std::string
  combineString1() const
  { return combineString1Max(); }

  virtual std::string
  finalizeString() const
  { return finalizeStringMax(); }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr** m_opands;
  int m_sz;
};


// ----------------------------------------------------------------------
// Mean
// ----------------------------------------------------------------------

class Mean
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  Mean(AExpr** oprnds, int numOprnds)
    : m_opands(oprnds), m_sz(numOprnds) 
  { }

  ~Mean();

  virtual double
  eval(const Metric::IData& mdata) const;

  virtual double
  evalNF(Metric::IData& mdata) const
  {
    double z = evalSum(mdata, m_opands, m_sz);
    accumVar(mdata) = z;
    return z;
  }


  // ------------------------------------------------------------
  // Metric::IDBExpr:
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { return m_sz; }


  virtual std::string
  combineString1() const
  { return combineString1Mean(); }

  virtual std::string
  finalizeString() const
  { return finalizeStringMean(); }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr** m_opands;
  int m_sz;
};


// ----------------------------------------------------------------------
// StdDev: standard deviation
// ----------------------------------------------------------------------

class StdDev
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  StdDev(AExpr** oprnds, int numOprnds)
    : m_opands(oprnds), m_sz(numOprnds) 
  { }

  ~StdDev();

  virtual double
  eval(const Metric::IData& mdata) const;

  virtual double
  evalNF(Metric::IData& mdata) const
  { return evalStdDevNF(mdata, m_opands, m_sz); }


  // ------------------------------------------------------------
  // Metric::IDBExpr: exported formulas for Flat and Callers view
  // ------------------------------------------------------------

  virtual bool
  hasAccum2() const
  { return true; }

  virtual uint
  numSrcFxd() const
  { return m_sz; }


  virtual std::string
  combineString1() const
  { return combineString1StdDev(); }

  virtual std::string
  combineString2() const
  { return combineString2StdDev(); }

  virtual std::string
  finalizeString() const
  { return finalizeStringStdDev(); }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr** m_opands;
  int m_sz;
};


// ----------------------------------------------------------------------
// CoefVar: relative standard deviation
// ----------------------------------------------------------------------

class CoefVar
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  CoefVar(AExpr** oprnds, int numOprnds)
    : m_opands(oprnds), m_sz(numOprnds) 
  { }

  ~CoefVar();

  virtual double
  eval(const Metric::IData& mdata) const;

  virtual double
  evalNF(Metric::IData& mdata) const
  { return evalStdDevNF(mdata, m_opands, m_sz); }


  // ------------------------------------------------------------
  // Metric::IDBExpr: exported formulas for Flat and Callers view
  // ------------------------------------------------------------

  virtual bool
  hasAccum2() const
  { return true; }

  virtual uint
  numSrcFxd() const
  { return m_sz; }


  virtual std::string
  combineString1() const
  { return combineString1StdDev(); }

  virtual std::string
  combineString2() const
  { return combineString2StdDev(); }

  virtual std::string
  finalizeString() const
  { return finalizeStringCoefVar(); }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr** m_opands;
  int m_sz;
};


// ----------------------------------------------------------------------
// RStdDev: relative standard deviation
// ----------------------------------------------------------------------

class RStdDev
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  RStdDev(AExpr** oprnds, int numOprnds)
    : m_opands(oprnds), m_sz(numOprnds)
  { }

  ~RStdDev();

  virtual double
  eval(const Metric::IData& mdata) const;

  virtual double
  evalNF(Metric::IData& mdata) const
  { return evalStdDevNF(mdata, m_opands, m_sz); }


  // ------------------------------------------------------------
  // Metric::IDBExpr: exported formulas for Flat and Callers view
  // ------------------------------------------------------------

  virtual bool
  hasAccum2() const
  { return true; }

  virtual uint
  numSrcFxd() const
  { return m_sz; }


  virtual std::string
  combineString1() const
  { return combineString1StdDev(); }

  virtual std::string
  combineString2() const
  { return combineString2StdDev(); }

  virtual std::string
  finalizeString() const
  { return finalizeStringRStdDev(); }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
  AExpr** m_opands;
  int m_sz;
};


// ----------------------------------------------------------------------
// NumSource
// ----------------------------------------------------------------------

class NumSource
  : public AExpr
{
public:
  // Assumes ownership of AExpr
  NumSource()
  { }

  ~NumSource()
  { }

  virtual double
  eval(const Metric::IData& mdata) const
  { DIAG_Die(DIAG_Unimplemented); return 0.0; }


  // ------------------------------------------------------------
  // Metric::IDBExpr: exported formulas for Flat and Callers view
  // ------------------------------------------------------------

  virtual uint
  numSrcFxd() const
  { DIAG_Die(DIAG_Unimplemented); return 0; }


  virtual std::string
  combineString1() const
  { return combineString1NumSource(); }

  virtual std::string
  finalizeString() const
  { return finalizeStringNumSource(); }


  // ------------------------------------------------------------
  // 
  // ------------------------------------------------------------

  virtual std::ostream&
  dumpMe(std::ostream& os = std::cout) const;

private:
};


//****************************************************************************

} // namespace Metric

} // namespace Prof

//****************************************************************************

#endif /* prof_Prof_Metric_AExpr_hpp */
