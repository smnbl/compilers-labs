#!/usr/bin/env bash
set -Eeuo pipefail

FILES="src/sema/scoperesolutionpass.hpp src/sema/typecheckingpass.cpp src/sema/scoperesolutionpass.cpp src/sema/typecheckingpass.hpp "

cd "$( dirname "${BASH_SOURCE[0]}" )"

tar -czf submission.tar.gz ${FILES}

echo "Tarball created. Upload the file 'submission.tar.gz' to Ufora."
