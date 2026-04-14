#!/bin/bash
targets=$1

echo ">>> START RUNNING TESTS"
echo "Arguments: $@"
echo "Directory: $(pwd)"

total=0
failed=0
for t in $targets; do
    test/$t
    n=$?
    if [[ $n -ne 0 ]]; then
        echo "-----------------"
        echo ">>> $t FAILED: $n"
        echo "-----------------"
        failed=$((failed + 1))
    else
        echo ">>> $t PASSED"
    fi
    total=$((total + 1))
done

echo ">>> Total: $total, Failed: $failed"
