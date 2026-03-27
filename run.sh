#!/bin/bash
# Copyright (c) 2026 MidnightHammer-code
# This source code is licensed under the GPL 3.0 license
# LICENSE file in the root directory of this source tree.

# Color Definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color (Reset)

ALL_PRESENT=true

for i in {0..8}; do
    FILE="pak00${i}.pk4"
    
    if [ -f "base/${FILE}" ]; then
        echo -e "[ ${GREEN}OK${NC} ] ${FILE}"
    else
        echo -e "[ ${RED}FAIL${NC} ] Missing: ${FILE}"
        ALL_PRESENT=false
    fi
done

echo "----------------------------------------"

if [ "$ALL_PRESENT" = true ]; then
    echo -e "${GREEN}Success! All required .pk4 files are present.${NC}"
    echo "Running game ..."
    echo "Happy Gaming"
    exit 0
else
    echo -e "${RED}Error: You are missing some .pk4 files. Please add all pk4 to the base/ folder.${NC}"
    exit 1
fi