[![Travis-CI: Linux/macOS build Status](https://travis-ci.com/dss-extensions/klusolve.svg?branch=master)](https://travis-ci.com/dss-extensions/klusolve)
[![AppVeyor: Windows build status](https://ci.appveyor.com/api/projects/status/k87v4udm4paiykap/branch/master?svg=true)](https://ci.appveyor.com/project/PMeira/klusolve/branch/master)

---

# DSS-Extensions: KLUSolve (KLUSolveX)

This is fork of KLUSolve used by DSS C-API. See the original at [SourceForge](https://sourceforge.net/p/klusolve/code/HEAD/tree/). As explicit in `LICENSE`, this work uses the same license, LGPL 2.1 or later.

Changes include:
- Uses a CMake building script.
- Uses either system SuiteSparse libraries (when compatible) or downloads upstream SuiteSparse source-code. The SuiteSparse source code is not included in the KLUSolveX repository anymore.
- Adds a dependency on [Eigen](http://eigen.tuxfamily.org/), dropping the customized CZSparse.
- Introduces some dense matrix functions (e.g. `mvmult`) to be used in the [DSS C-API](https://github.com/dss-extensions/dss_capi/), an alternative [OpenDSS](sf.net/p/electricdss) library, for better performance.
- Introduces reuse of the symbolic and numeric factorization steps from KLU when the sparse matrix is unchanged.

For binary distributions, basic descriptions of the dependencies and licensing information is reproduced below. When building from source, be sure to check the licenses of the components.

**Currently tested with Eigen 3.3.7 and SuiteSparse 5.6.0.**

---

KLUSolveX
Copyright (c) 2017-2019, Paulo Meira
Copyright (c) 2008, EnerNex Corporation
All rights reserved.

KLUSolveX is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

KLUSolveX is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this Module; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

---

[Eigen](http://eigen.tuxfamily.org/) is Free Software. Starting from the 3.1.1 version, it is licensed under the MPL2, which is a simple weak copyleft license. Common questions about the MPL2 are answered in the official [MPL2 FAQ](http://www.mozilla.org/MPL/2.0/FAQ.html).

Note that currently, a few features rely on third-party code licensed under the LGPL: SimplicialCholesky, AMD ordering, and constrained_cg. Such features can be explicitly disabled by compiling with the EIGEN_MPL2_ONLY preprocessor symbol defined. Furthermore, Eigen provides interface classes for various third-party libraries (usually recognizable by the <Eigen/*Support> header name). Of course you have to mind the license of the so-included library when using them.

Virtually any software may use Eigen. For example, closed-source software may use Eigen without having to disclose its own source code. Many proprietary and closed-source software projects are using Eigen right now, as well as many BSD-licensed projects. 

---

AMD, Copyright (c) 2009-2013 by Timothy A. Davis (http://www.suitesparse.com),
Patrick R. Amestoy, and Iain S. Duff.  All Rights Reserved.  AMD is available
under alternate licences; contact T. Davis for details.

AMD:  a set of routines for permuting sparse matrices prior to
    factorization.  Includes a version in C, a version in Fortran, and a MATLAB
    mexFunction.
   
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.
        * Neither the name of the organizations to which the authors are
          affiliated, nor the names of its contributors may be used to endorse
          or promote products derived from this software without specific prior
          written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
    DAMAGE.

   
---


COLAMD, Copyright 1998-2016, Timothy A. Davis.  http://www.suitesparse.com
-------------------------------------------------------------------------------

The COLAMD column approximate minimum degree ordering algorithm computes
a permutation vector P such that the LU factorization of A (:,P)
tends to be sparser than that of A.  The Cholesky factorization of
(A (:,P))'*(A (:,P)) will also tend to be sparser than that of A'*A.
SYMAMD is a symmetric minimum degree ordering method based on COLAMD,
available as a MATLAB-callable function.  It constructs a matrix M such
that M'*M has the same pattern as A, and then uses COLAMD to compute a column
ordering of M.  Colamd and symamd tend to be faster and generate better
orderings than their MATLAB counterparts, colmmd and symmmd.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.
        * Neither the name of the organizations to which the authors are
          affiliated, nor the names of its contributors may be used to endorse
          or promote products derived from this software without specific prior
          written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
    DAMAGE.

---

BTF, by Timothy A. Davis, Copyright (C) 2004-2016, University of Florida
BTF is also available under other licenses; contact the author for details.
http://www.suitesparse.com

BTF is a software package for permuting a matrix into block upper triangular
form.  It includes a maximum transversal algorithm, which finds a permutation
of a square or rectangular matrix so that it has a zero-free diagonal (if one
exists); otherwise, it finds a maximal matching which maximizes the number of
nonzeros on the diagonal.  The package also includes a method for finding the
strongly connected components of a graph.  These two methods together give the
permutation to block upper triangular form.

BTF is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

BTF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this Module; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

---

KLU, Copyright (C) 2004-2013, University of Florida
by Timothy A. Davis and Ekanathan Palamadai.
KLU is also available under other licenses; contact authors for details.
http://www.suitesparse.com

KLU is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

KLU is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this Module; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

---
