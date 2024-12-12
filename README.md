[![Builds](https://github.com/dss-extensions/klusolve/actions/workflows/builds.yml/badge.svg)](https://github.com/dss-extensions/klusolve/actions/workflows/builds.yml)

---

# DSS-Extensions: KLUSolveX

This is a fork of **KLUSolve** initially intended for AltDSS/DSS C-API.
See the original at [SourceForge](https://sourceforge.net/p/klusolve/code/HEAD/tree/). As explicit in `LICENSE`, this work uses the same license, LGPL 2.1 or later.

In version 1.1, we restored compatibility for Windows x86 (32-bit) builds, using the same calling convention as the original KLUSolve (`__stdcall`). The other builds were already compatible.

Changes include:
- Uses a CMake building script. **Note**: this was added long before CMake was used in the original KLUSolve or SuiteSparse. Our script is not based on the one in the KLUSolve repository, and it is more general, resulting in a cleaner repository.
- Uses either system SuiteSparse libraries (when compatible) or downloads upstream SuiteSparse source-code. The SuiteSparse source code is not included in the KLUSolveX repository anymore.
- Adds a dependency on [Eigen](http://eigen.tuxfamily.org/), dropping the customized CZSparse.
- Introduces some dense matrix functions (e.g. `mvmult`) to be used in the [DSS C-API](https://github.com/dss-extensions/dss_capi/), an alternative [OpenDSS](sf.net/p/electricdss) library, for better performance.
- Introduces reuse of the symbolic and numeric factorization steps from KLU when the sparse matrix is unchanged.

For binary distributions, basic descriptions of the dependencies and licensing information is reproduced below. When building from source, be sure to check the licenses of the components.

**Currently tested with Eigen 3.4.0 and SuiteSparse 5.6.0.** KLU, which is distributed in SuiteSparse, is very stable, so this library should be forward compatible. We will eventually integrate with the current SuiteSparse (CMake) build setup, but for now our build scripts provide a more streamlined setup when building from source.

# Credits / Acknowledgment

Original version is at [SourceForge](https://sourceforge.net/p/klusolve/code/HEAD/tree/). 

Most of the changes and new features were contributed by [Paulo Meira (@PMeira)](https://github.com/PMeira).

Thanks for [Felipe Markson (@felipemarkson)](https://github.com/felipemarkson) for contributing a static build option.

Since we don't include the code for KLU or SuiteSparse in this repository anymore, please see [SuiteSparse](https://people.engr.tamu.edu/davis/suitesparse.html) and [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) for credits and details of the dependencies.

---

KLUSolveX

Copyright (c) 2017-2024, Paulo Meira

Copyright (c) 2019-2024, DSS Extensions contributors

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

