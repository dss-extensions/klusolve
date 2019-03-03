# DSS-Extensions: KLUSolve1

This is a modified version of KLUSolve, see the original at [SourceForge](https://sourceforge.net/p/klusolve/code/HEAD/tree/). As explicit in `LICENSE`, this work uses the same license, LGPL 2.1 or later.

Changes include:
- Uses a CMake building script
- Optionally use system SuiteSparse libraries
- Download upstream SuiteSparse source-code instead
- Adds a dependency on [Eigen](http://eigen.tuxfamily.org/)
- Introduces some dense matrix functions (e.g. `mvmult`) to be used in the [DSS C-API](https://github.com/dss-extensions/dss_capi/), an alternative [OpenDSS](sf.net/p/electricdss) library, for better performance.

The original README is updated and follows.

==========================================================================

KLU, Copyright (C) 2004-2013, University of Florida
by Timothy A. Davis and Ekanathan Palamadai.
KLU is also available under other licenses; contact authors for details.
http://www.suitesparse.com

CSparse: a Concise Sparse Matrix package.
Version 2.2.0, Copyright (c) 2006-2007, Timothy A. Davis, Mar 31, 2007.

AMD Version 2.2, Copyright (c) 2007 by Timothy A.
Davis, Patrick R. Amestoy, and Iain S. Duff.  All Rights Reserved.

BTF Version 1.0, May 31, 2007, by Timothy A. Davis
Copyright (C) 2004-2007, University of Florida

CZSparse, Copyright (c) 2008, EnerNex Corporation. All rights reserved.

==========================================================================

KLUSolve, Copyright (c) 2008, EnerNex Corporation
Copyright (c) 2017-2019, Paulo Meira
All rights reserved.

KLUSolve is a complex sparse matrix library tailored to electric power 
systems, licensed under LGPL version 2.1 (see files License.txt and 
lgpl_2_1.txt in this directory). For an example of use, see the OpenDSS 
project at https://sf.net/p/electricdss or the DSS C-API library
at https://github.com/dss-extensions/dss_capi

==========================================================================

KLUSolve is based on the KLU, CSparse, and supporting libraries developed
by Timothy A. Davis and his students, which are also licensed under LGPL 
version 2.1.  All source files used in KLUSolve are included in the 
SourceForge SVN repository for KLUSolve

Current versions: http://faculty.cse.tamu.edu/davis/suitesparse.html

Contact: davis at tamu dot edu, or 

  Tim Davis, Professor
  425E HRBB
  Texas A&M University
  College Station, TX 77843-3112
  1-979-845-4094

Reference Book: "Direct Methods for Sparse Linear Systems," Timothy A. Davis,
SIAM, Philadelphia, 2006.

==========================================================================

[Eigen](http://eigen.tuxfamily.org/) is Free Software. Starting from the 3.1.1 version, it is licensed under the MPL2, which is a simple weak copyleft license. Common questions about the MPL2 are answered in the official [MPL2 FAQ](http://www.mozilla.org/MPL/2.0/FAQ.html).

Note that currently, a few features rely on third-party code licensed under the LGPL: SimplicialCholesky, AMD ordering, and constrained_cg. Such features can be explicitly disabled by compiling with the EIGEN_MPL2_ONLY preprocessor symbol defined. Furthermore, Eigen provides interface classes for various third-party libraries (usually recognizable by the <Eigen/*Support> header name). Of course you have to mind the license of the so-included library when using them.

Virtually any software may use Eigen. For example, closed-source software may use Eigen without having to disclose its own source code. Many proprietary and closed-source software projects are using Eigen right now, as well as many BSD-licensed projects. 
