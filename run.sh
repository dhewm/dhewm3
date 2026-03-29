#!/bin/bash
# Copyright (c) 2026 MidnightHammer-code
# This source code is licensed under the GPL 3.0 license
# LICENSE file in the root directory of this source tree.

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
NONE='\033[0m'

ALL_PRESENT=true

for i in {0..8}; do
    FILE="pak00${i}.pk4"
    
    if [ -f "base/${FILE}" ]; then
        echo -e "[ ${GREEN}OK${NONE} ] ${FILE}"
    else
        echo -e "[ ${RED}FAIL${NONE} ] Missing: ${FILE}"
        ALL_PRESENT=false
    fi
done

echo "----------------------------------------"

if [ "$ALL_PRESENT" = true ]; then
    echo -e "${GREEN}Success! All required .pk4 files are present.${NONE}"
    echo "Running game ..."
    echo "Happy Gaming"
    exit 0
else
    echo -e "${RED}Error: You are missing some .pk4 files. Please add all pk4 to the base/ folder.${NONE}"
    exit 1
fi
