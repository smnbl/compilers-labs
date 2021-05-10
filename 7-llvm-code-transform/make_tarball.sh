#!/usr/bin/env bash
set -Eeuo pipefail

FILES=" REPORT.pdf"

cd "$( dirname "${BASH_SOURCE[0]}" )"

tar -czf submission.tar.gz ${FILES}

echo "Tarball created. Upload the file 'submission.tar.gz' to Ufora."
