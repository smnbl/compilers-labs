#!/usr/bin/env bash
set -Eeuo pipefail

FILES="src/codegen-x64/optimise-x64.cpp src/codegen-x64/codegen-x64.hpp src/codegen-x64/optimise-x64.hpp src/codegen-x64/codegen-x64.cpp  REPORT.pdf"

cd "$( dirname "${BASH_SOURCE[0]}" )"

tar -czf submission.tar.gz ${FILES}

echo "Tarball created. Upload the file 'submission.tar.gz' to Ufora."
