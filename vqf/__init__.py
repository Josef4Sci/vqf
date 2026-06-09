# SPDX-FileCopyrightText: 2021 Daniel Laidig <laidig@control.tu-berlin.de>
#
# SPDX-License-Identifier: MIT

import sys
from pathlib import Path


def _rebuild_if_stale():
    pkg = Path(__file__).parent
    cpp_sources = list((pkg / 'cpp').glob('*.cpp')) + list((pkg / 'cpp').glob('*.hpp'))
    pyd_files = list(pkg.glob('vqf.*.pyd')) + list(pkg.glob('vqf.*.so'))

    if not cpp_sources:
        return

    cpp_newest = max(f.stat().st_mtime for f in cpp_sources)
    pyd_oldest = min((f.stat().st_mtime for f in pyd_files), default=0.0)

    if cpp_newest > pyd_oldest:
        import subprocess
        print('[vqf] C++ sources changed, rebuilding...', file=sys.stderr)
        result = subprocess.run(
            [sys.executable, 'setup.py', 'build_ext', '--inplace'],
            cwd=str(pkg.parent), capture_output=True, text=True
        )
        if result.returncode != 0:
            print(result.stdout, file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            raise RuntimeError('[vqf] Auto-rebuild failed — see errors above')


_rebuild_if_stale()

try:
    from .vqf import VQF, offlineVQF
except ModuleNotFoundError:
    VQF = None
    offlineVQF = None
from .basicvqf import BasicVQF
from .pyvqf import PyVQF
from .utils import get_cpp_path, get_matlab_path

__all__ = ['VQF', 'BasicVQF', 'offlineVQF', 'PyVQF', 'get_cpp_path', 'get_matlab_path']
