#!/bin/bash

# Comprehensive test for Linux password hashing (MD5, SHA-256, SHA-512)
# This script covers ALL cases that Linux uses for password hashing

# set -e

BINARY="./naucanh"
PASSED=0
FAILED=0

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}=== COMPREHENSIVE Linux Password Hash Testing ===${NC}"
echo "Testing MD5-crypt, SHA-256-crypt, SHA-512-crypt implementations"
echo ""

# Check dependencies
check_dependencies() {
    echo -e "${BLUE}Checking dependencies...${NC}"
    
    if [ ! -f "$BINARY" ]; then
        echo -e "${RED}❌ Binary $BINARY not found. Run 'make' first.${NC}"
        exit 1
    fi
    
    if ! command -v python3 &> /dev/null; then
        echo -e "${RED}❌ python3 not found${NC}"
        exit 1
    fi
    
    # Check if python3 has crypt module
    if ! python3 -c "import crypt" 2>/dev/null; then
        echo -e "${RED}❌ Python crypt module not available${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✅ All dependencies available${NC}"
    echo ""
}

# Test function
test_case() {
    local type="$1"
    local salt="$2"
    local password="$3"
    local description="$4"
    local test_number="$5"
    
    echo -e "${YELLOW}Test $test_number: $description${NC}"
    echo "  Algorithm: $type, Salt: '$salt', Password: '$password'"
    
    # Build full salt string based on type
    local full_salt=""
    case "$type" in
        "-1") full_salt="\$1\$$salt" ;;
        "-5") full_salt="\$5\$$salt" ;;
        "-6") full_salt="\$6\$$salt" ;;
    esac
    
    # Get system reference hash
    local system_hash
    system_hash=$(python3 -c "
import crypt
import sys
try:
    result = crypt.crypt('$password', '$full_salt')
    print(result)
except Exception as e:
    print('ERROR: ' + str(e))
    sys.exit(1)
")
    
    # Get our implementation result
    local our_result
    our_result=$($BINARY "$type" "$salt" "$password" 2>/dev/null || echo "ERROR")
    
    echo "  System hash:    $system_hash"
    echo "  Our result:     $our_result"
    
    # Compare results
    if [ "$our_result" = "$system_hash" ]; then
        echo -e "  ${GREEN}✅ PASSED${NC}"
        ((PASSED++))
    else
        echo -e "  ${RED}❌ FAILED${NC}"
        echo -e "  ${RED}MISMATCH DETECTED!${NC}"
        ((FAILED++))
    fi
    echo ""
}

# Test error cases
test_error_case() {
    local args="$1"
    local description="$2"
    local expected_behavior="$3"
    
    echo -e "${YELLOW}Error Test: $description${NC}"
    echo "  Command: $BINARY $args"
    echo "  Expected: $expected_behavior"
    
    local result
    result=$($BINARY $args 2>&1 || echo "COMMAND_FAILED")
    
    echo "  Result: $result"
    
    case "$expected_behavior" in
        "usage_error")
            if [[ "$result" == *"Usage"* ]] || [[ "$result" == "COMMAND_FAILED" ]]; then
                echo -e "  ${GREEN}✅ PASSED${NC}"
                ((PASSED++))
            else
                echo -e "  ${RED}❌ FAILED${NC}"
                ((FAILED++))
            fi
            ;;
        "unsupported_error")
            if [[ "$result" == *"Unsupported"* ]] || [[ "$result" == "COMMAND_FAILED" ]]; then
                echo -e "  ${GREEN}✅ PASSED${NC}"
                ((PASSED++))
            else
                echo -e "  ${RED}❌ FAILED${NC}"
                ((FAILED++))
            fi
            ;;
    esac
    echo ""
}

# Start testing
check_dependencies

echo -e "${CYAN}=== 1. MD5-CRYPT TESTING (RFC 1321 based) ===${NC}"
echo "MD5-crypt format: \$1\$salt\$hash"
echo "Used by: Old Linux systems, some embedded systems"
echo ""

# MD5-crypt test cases
test_case "-1" "testsalt" "password" "Basic MD5-crypt" "1.1"
test_case "-1" "salt" "hello" "Simple MD5-crypt" "1.2"
test_case "-1" "abcdefgh" "123456" "MD5-crypt with 8-char salt" "1.3"
test_case "-1" "a" "test" "MD5-crypt with minimal salt" "1.4"
test_case "-1" "12345678" "password123" "MD5-crypt with numeric salt" "1.5"
test_case "-1" "mysalt" "" "MD5-crypt with empty password" "1.6"
test_case "-1" "salt" "p@ssw0rd!" "MD5-crypt with special chars" "1.7"
test_case "-1" "verylongsalt123" "short" "MD5-crypt with long salt (truncated)" "1.8"
test_case "-1" "test" "verylongpasswordwithmanycharacters12345678901234567890" "MD5-crypt with long password" "1.9"

# Known MD5-crypt vectors
echo -e "${BLUE}Known MD5-crypt test vectors:${NC}"
test_case "-1" "deadbeef" "mypassword" "Known vector 1" "1.10"
test_case "-1" "strangersalt" "password" "Known vector 2" "1.11"

echo -e "${CYAN}=== 2. SHA-256-CRYPT TESTING (FIPS 180-2 based) ===${NC}"
echo "SHA-256-crypt format: \$5\$salt\$hash"
echo "Used by: Modern Linux systems (default on many distros)"
echo ""

# SHA-256-crypt test cases
test_case "-5" "testsalt" "password" "Basic SHA-256-crypt" "2.1"
test_case "-5" "salt" "hello" "Simple SHA-256-crypt" "2.2"
test_case "-5" "rounds=1000\$salt" "password" "SHA-256 with explicit rounds" "2.3"
test_case "-5" "longsalt1234567890abcdef" "password123" "SHA-256 with long salt" "2.4"
test_case "-5" "a" "test" "SHA-256 with minimal salt" "2.5"
test_case "-5" "12345678901234567890123456789012" "password" "SHA-256 with max salt length" "2.6"
test_case "-5" "mysalt" "" "SHA-256 with empty password" "2.7"
test_case "-5" "salt" "unicode🔒test" "SHA-256 with unicode password" "2.8"
test_case "-5" "test" "verylongpasswordwithmanycharacters12345678901234567890abcdefghijklmnopqrstuvwxyz" "SHA-256 with very long password" "2.9"

# Common SHA-256 patterns
echo -e "${BLUE}Common SHA-256-crypt patterns:${NC}"
test_case "-5" "randomsalt" "admin" "Common password 'admin'" "2.10"
test_case "-5" "usersalt" "user" "Common password 'user'" "2.11"
test_case "-5" "rootsalt" "root" "Common password 'root'" "2.12"

echo -e "${CYAN}=== 3. SHA-512-CRYPT TESTING (FIPS 180-2 based) ===${NC}"
echo "SHA-512-crypt format: \$6\$salt\$hash"
echo "Used by: Modern Linux systems (preferred for security)"
echo ""

# SHA-512-crypt test cases - including the critical one
test_case "-6" "1CCHezNlaiEUpRNw" "1231234" "CRITICAL: Original test vector" "3.1"
test_case "-6" "testsalt" "password" "Basic SHA-512-crypt" "3.2"
test_case "-6" "salt" "hello" "Simple SHA-512-crypt" "3.3"
test_case "-6" "rounds=5000\$salt" "password" "SHA-512 with explicit rounds" "3.4"
test_case "-6" "longsalt1234567890abcdef" "password123" "SHA-512 with long salt" "3.5"
test_case "-6" "a" "test" "SHA-512 with minimal salt" "3.6"
test_case "-6" "verylongsalt1234567890abcdefghijklmnopqrstuvwxyz" "password" "SHA-512 with very long salt" "3.7"
test_case "-6" "mysalt" "" "SHA-512 with empty password" "3.8"
test_case "-6" "salt" "p@ssw0rd!@#\$%^&*()_+" "SHA-512 with all special chars" "3.9"
test_case "-6" "test" "verylongpasswordwithmanycharacters12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" "SHA-512 with very long password" "3.10"

# Real-world SHA-512 patterns
echo -e "${BLUE}Real-world SHA-512-crypt patterns:${NC}"
test_case "-6" "randomsalt123" "Password123!" "Complex password pattern" "3.11"
test_case "-6" "systemsalt" "changeme" "Default password pattern" "3.12"
test_case "-6" "usersalt456" "qwerty" "Common weak password" "3.13"

echo -e "${CYAN}=== 4. BOUNDARY AND EDGE CASES ===${NC}"

# Salt boundary tests
echo -e "${BLUE}Salt boundary tests:${NC}"
test_case "-1" "" "password" "MD5 with empty salt" "4.1"
test_case "-5" "" "password" "SHA-256 with empty salt" "4.2"
test_case "-6" "" "password" "SHA-512 with empty salt" "4.3"

# Maximum salt length tests (Linux crypt typically limits salt)
test_case "-1" "12345678" "password" "MD5 with 8-char salt (max)" "4.4"
test_case "-5" "1234567890123456" "password" "SHA-256 with 16-char salt" "4.5"
test_case "-6" "1234567890123456" "password" "SHA-512 with 16-char salt" "4.6"

# Password boundary tests
echo -e "${BLUE}Password boundary tests:${NC}"
test_case "-1" "salt" "a" "Single character password" "4.7"
test_case "-5" "salt" "a" "Single character password SHA-256" "4.8"
test_case "-6" "salt" "a" "Single character password SHA-512" "4.9"

# Binary data simulation (printable chars that might cause issues)
echo -e "${BLUE}Special character tests:${NC}"
test_case "-1" "salt" "passwd\$with\$dollars" "Password with dollar signs" "4.10"
test_case "-5" "salt" "passwd:with:colons" "Password with colons" "4.11"
test_case "-6" "salt" "passwd with spaces" "Password with spaces" "4.12"

echo -e "${CYAN}=== 5. ERROR HANDLING TESTS ===${NC}"

# Wrong number of arguments
test_error_case "" "No arguments" "usage_error"
test_error_case "-6" "Only one argument" "usage_error"
test_error_case "-6 salt" "Only two arguments" "usage_error"
test_error_case "-6 salt pass extra" "Too many arguments" "usage_error"

# Unsupported algorithms
test_error_case "-2 salt password" "Unsupported algorithm -2" "unsupported_error"
test_error_case "-7 salt password" "Unsupported algorithm -7" "unsupported_error"
test_error_case "-0 salt password" "Unsupported algorithm -0" "unsupported_error"
test_error_case "md5 salt password" "Invalid algorithm format" "unsupported_error"

echo -e "${CYAN}=== 6. PERFORMANCE AND STRESS TESTS ===${NC}"

echo -e "${BLUE}Performance test: 100 hashes of each type${NC}"
start_time=$(date +%s.%N)

# MD5 performance
for i in {1..100}; do
    $BINARY "-1" "perf$i" "testpass$i" > /dev/null
done
md5_time=$(date +%s.%N)

# SHA-256 performance  
for i in {1..100}; do
    $BINARY "-5" "perf$i" "testpass$i" > /dev/null
done
sha256_time=$(date +%s.%N)

# SHA-512 performance
for i in {1..100}; do
    $BINARY "-6" "perf$i" "testpass$i" > /dev/null
done
end_time=$(date +%s.%N)

md5_duration=$(echo "$md5_time - $start_time" | bc -l)
sha256_duration=$(echo "$sha256_time - $md5_time" | bc -l)
sha512_duration=$(echo "$end_time - $sha256_time" | bc -l)

echo "  MD5-crypt:     ${md5_duration}s ($(echo "scale=4; $md5_duration / 100" | bc -l)s per hash)"
echo "  SHA-256-crypt: ${sha256_duration}s ($(echo "scale=4; $sha256_duration / 100" | bc -l)s per hash)"
echo "  SHA-512-crypt: ${sha512_duration}s ($(echo "scale=4; $sha512_duration / 100" | bc -l)s per hash)"
echo ""

echo -e "${CYAN}=== 7. COMPATIBILITY VERIFICATION ===${NC}"

# Create test vectors and verify against system
echo -e "${BLUE}Verifying compatibility with system crypt()...${NC}"

# Test common Linux user scenarios
common_passwords=("password" "123456" "admin" "root" "user" "test" "changeme" "qwerty" "letmein" "welcome")
common_salts=("salt" "test" "user" "sys" "random" "abcd" "1234" "xyz" "abc" "def")

echo "Testing ${#common_passwords[@]} passwords × ${#common_salts[@]} salts × 3 algorithms = $((${#common_passwords[@]} * ${#common_salts[@]} * 3)) combinations"

compat_passed=0
compat_failed=0

for password in "${common_passwords[@]}"; do
    for salt in "${common_salts[@]}"; do
        for type in "-1" "-5" "-6"; do
            case "$type" in
                "-1") full_salt="\$1\$$salt" ;;
                "-5") full_salt="\$5\$$salt" ;;
                "-6") full_salt="\$6\$$salt" ;;
            esac
            
            our_result=$($BINARY "$type" "$salt" "$password" 2>/dev/null || echo "ERROR")
            system_result=$(python3 -c "import crypt; print(crypt.crypt('$password', '$full_salt'))" 2>/dev/null || echo "ERROR")
            
            if [ "$our_result" = "$system_result" ]; then
                ((compat_passed++))
            else
                ((compat_failed++))
                echo -e "${RED}COMPAT FAIL: $type $salt $password${NC}"
                echo "  Our:    $our_result"
                echo "  System: $system_result"
            fi
        done
    done
done

echo -e "${BLUE}Compatibility results: $compat_passed passed, $compat_failed failed${NC}"
echo ""

# Final results
echo -e "${CYAN}=== FINAL RESULTS ===${NC}"
total_tests=$((PASSED + FAILED))
echo -e "Total tests run: $total_tests"
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"

if [ $FAILED -eq 0 ] && [ $compat_failed -eq 0 ]; then
    echo -e "${GREEN}🎉 ALL TESTS PASSED! 🎉${NC}"
    echo -e "${GREEN}Your implementation is 100% compatible with Linux password hashing!${NC}"
    exit 0
else
    echo -e "${RED}❌ SOME TESTS FAILED${NC}"
    echo -e "${RED}Implementation needs fixes for full Linux compatibility${NC}"
    exit 1
fi