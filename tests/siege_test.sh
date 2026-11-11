#!/bin/bash

# Settings
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

SERVER="http://localhost:8080"
SERVER1="http://localhost:9080"
SERVER2="http://localhost:9090"
SERVER3="http://localhost:6080"

PASS=0
FAIL=0

##################
#     RESULTS    #
##################

show_results() {
echo -e "\n=============================="
echo "SUMMARY"
echo "=============================="

printf "\e[32m"
echo "PASS: $PASS"
printf "\e[0m"

printf "\e[31m"
echo "FAIL: $FAIL"
printf "\e[0m"
printf "\e[1m"
echo "TOTAL: $((PASS + FAIL))"
printf "\e[0m"

if [ "$FAIL" -eq 0 ]; then
  EXIT_CODE=0
else
  EXIT_CODE=1
fi

if [ "$EXIT_CODE" -eq 0 ]; then

	printf "\e[32m"

	cat << 'EOF'
  ____                              
 / ___| _   _  ___ ___ ___  ___ ___ 
 \___ \| | | |/ __/ __/ _ \/ __/ __|
  ___) | |_| | (_| (_|  __/\__ \__ \
 |____/ \__,_|\___\___\___||___/___/
                                    
EOF

	printf "\e[0m\n"

else

	printf "\e[31m"

	cat << 'EOF'
  _____                               _       
 |_   _| __ _   _    __ _  __ _  __ _(_)_ __  
   | || '__| | | |  / _` |/ _` |/ _` | | '_ \ 
   | || |  | |_| | | (_| | (_| | (_| | | | | |
   |_||_|   \__, |  \__,_|\__, |\__,_|_|_| |_|
            |___/         |___/               

EOF

	printf "\e[0m\n"

fi
}


##################
#     HELPERES   #
##################

log() {
    echo -e "\n=============================="
    echo "$1"
    echo "=============================="
}

run() {
  local name="$1"
  local cmd="$2"

  printf "\e[33m"
  echo -e "\n[RUN] $name"
  printf "\e[0m\n"

  # siege command
  local output
  output=$(timeout --foreground -k 5s 90s bash -c "$cmd" 2>&1)
  rc=$?

  echo "$output"

  if [ "$rc" -eq 124 ]; then
      printf "\e[31m"
      echo "[TIMEOUT] $name"
      printf "\e[0m"

      FAIL=$((FAIL+1))
      show_results
      exit 1
  fi

  if echo "$output" | grep -q -i "availability" && echo "$output" | grep -i "availability" | grep -qE ':[[:space:]]*"?0\.00'; then
    local status=1
  else 
    local status=0
  fi

  if [ "$status" -eq 0 ]; then
    printf "\e[32m"
    echo "[OK] $name"
    printf "\e[0m\n"

    echo "=============================="

    PASS=$((PASS+1))
  else
    printf "\e[31m"
    echo "[FAIL] $name"
    FAIL=$((FAIL+1))
    printf "\e[0m\n"
    echo "=============================="

    show_results
    exit 1
  fi
}

##################
#     SET-UP     #
##################
mkdir -p www
mkdir -p www/some_directory

[ -f www/empty.html ] || touch www/empty.html
[ -f www/index.html ] || echo "<html>index</html>" > www/index.html
[ -f www/notfound ] && rm -f www/notfound
[ -d www/some_directory ] || mkdir -p www/some_directory

##################
#      TESTS     #
##################

# =================================================================
# GET TESTS
# =================================================================

# Test 1
# siege -c1 -r1 http://localhost:8080/
test_basic_get() {
  run "Test 1: GET single request" "siege -c1 -r1 $SERVER/"
}

# Test 2
# siege -t10S http://localhost:8080/
test_sustained_load() {
  run "Test 2: Load 10s" "siege -t10S $SERVER/"
}

# Test 3
# siege -c5 -t10S http://localhost:8080/
test_homepage() {
  run "Test 3: Homepage load" "siege -c5 -t10S $SERVER/"
}

# Test 4
# siege -c10 -t60S http://localhost:8080/
test_long_load() {
  run "Test 4: Stability test" "siege -c10 -t60S $SERVER/"
}

# Test 5
# siege -c10 -r10 http://localhost:8080/index.html
test_static_file() {
  run "Test 5: Static file" "siege -c10 -r10 $SERVER/index.html"
}

# Test 6
# siege -c10 -r5 http://localhost:8080/notfound
test_404() {
  run "Test 6: 404 test" "siege -c10 -r5 $SERVER/notfound"
}

# Test 7
# siege -c5 -r5 http://localhost:8080/doesnotexist
test_error_page() {
  run "Test 7: Error page" "siege -c5 -r5 $SERVER/doesnotexist"
}

# Test 8
# siege -c20 -t15S -H 'Connection: keep-alive' http://localhost:8080/
test_keep_alive() {
  run "Test 8: Keep alive" "siege -c20 -t15S -H 'Connection: keep-alive' $SERVER/"
}

# Test 9
# siege -c10 -t10s http://localhost:8080/some_directory/
test_autoindex() {
  run "Test 9: Autoindex" "siege -c10 -t10s $SERVER/some_directory/"
}

# Test 10
#siege -b -c30 -t10S http://localhost:8080/
test_get_stress() {
  log "Test 10: GET stress empty"

  output=$(timeout --foreground -k 5s 30s \
    siege -b -c30 -t10S "$SERVER/" 2>&1)

  local rc=$?

  if [ "$rc" -eq 124 ]; then
      echo -e "\e[31m[TIMEOUT] Stress test\e[0m"
      FAIL=$((FAIL+1))
      show_results
      exit 1
  fi

  echo "$output"

  availability=$(echo "$output" | grep -i "availability" | awk -F: '{print $2}' | tr -d ' \t,%\"')

  if [ -z "$availability" ] || [ "$availability" = "0.00" ] || (( $(echo "$availability < 99.5" | bc -l) )); then
    echo -e "\e[31m[FAIL] availability too low or server down: ${availability:-0}%\e[0m"
    FAIL=$((FAIL+1))
    show_results
    exit 1
  else
    echo -e "\e[32m[OK] availability: $availability%\e[0m"
    PASS=$((PASS+1))
  fi
}

# Test 11
#siege -b -c30 -t10S http://localhost:8080/empty.html
test_empty_page() {
  log "Test 11: GET Empty page"

  output=$(timeout --foreground -k 5s 40s \
      siege -b -c30 -t10S "$SERVER/empty.html" 2>&1)

  rc=$?

  if [ "$rc" -eq 124 ]; then
      echo -e "\e[31m[TIMEOUT] Empty page test\e[0m"
      FAIL=$((FAIL+1))
      show_results
      exit 1
  fi

  echo "$output"

  availability=$(echo "$output" | grep -i "availability" | awk -F: '{print $2}' | tr -d ' \t,%\"')

  if [ -z "$availability" ] || (( $(echo "$availability < 99.5" | bc -l) )); then
      echo -e "\e[31m[FAIL] Empty page availability: ${availability:-0}%\e[0m"
      FAIL=$((FAIL+1))
      show_results
      exit 1
  else
      echo -e "\e[32m[OK] Empty page availability: $availability%\e[0m"
      PASS=$((PASS+1))
  fi
}

# =================================================================
# POST TESTS
# =================================================================

# Test 12
#siege -c1 -r1 'http://localhost:8080/upload/test.txt POST data=hello'
test_post_single() {
  run "Test 12: POST single request" "siege -c1 -r1 '$SERVER/upload/test.txt POST data=hello'"
}

# Test 13
#siege -c10 -t10S 'http://localhost:8080/upload/test.txt POST data=hello'
test_post_sustained() {
  run "Test 13: POST load 10s" "siege -c10 -t10S '$SERVER/upload/test.txt POST data=hello'"
}

# Test 14
#siege -c50 -t15S 'http://localhost:8080/upload/test.txt POST data=hello'
test_post_heavy() {
  run "Test 14: POST high concurrency" "siege -c50 -t15S '$SERVER/upload/test.txt POST data=hello'"
}

# Test 15
#siege -c10 -r5 'http://localhost:8080/ POST data=hello'
test_post_root() {
  run "Test 15: POST to root directory" "siege -c10 -r5 '$SERVER/ POST data=hello'"
}

# Test 16
#siege -c10 -r5 'http://localhost:8080/notfound POST data=hello'
test_post_404() {
  run "Test 16: POST to non-existent file" "siege -c10 -r5 '$SERVER/notfound POST data=hello'"
}

# Test 17
#siege -b -c30 -t15S -H 'Connection: keep-alive' 'http://localhost:8080/upload/test.txt POST data=hello'
test_post_keep_alive() {
  run "Test 17: POST Keep-Alive stress" "siege -b -c30 -t15S -H 'Connection: keep-alive' '$SERVER/upload/test.txt POST data=hello'"
}


# =================================================================
# OTHER TESTS
# =================================================================

# Test 18: Redirection Stress Test
#siege -c40 -t10S http://localhost:8080/redirect-me
test_redirect_stress() {
  run "Test 18: REDIRECT high concurrency 10s" "siege -c40 -t10S $SERVER/redirect-me"
}

# Test 19: DELETE Stress Test
#siege -c10 -r5 'http://localhost:8080/uploads/delete_test_1.txt DELETE'
test_delete_stress() {
  log "Test 19: Preparing files for DELETE stress"
  
  # Create 50 files that will be deleted
  for i in {1..50}; do
    touch "www/uploads/delete_test_${i}.txt"
  done

  run "Test 19: DELETE high concurrency" "siege -c10 -r5 '$SERVER/uploads/delete_test_1.txt DELETE'"
}

# Test 20: Concurrent Big Files POST Stress
#siege -c5 -r3 'http://localhost:8080/upload/big_post_file.bin POST'
test_post_big_files_stress() {
  log "Test 20: Preparing 5MB file for big POST stress"
  
  # Create file
  dd if=/dev/urandom of=www/upload-big/big_post_file.bin bs=1M count=5 2>/dev/null

  #'-F' sends the real body file
  run "Test 20: Concurrent Big POST files" "siege -c5 -r3 'http://localhost:8080/upload/big_post_file.bin POST'"
  
  # Clean-up local file
  rm -f www/upload/big_post_file.bin
}

# Test 21: Multi-CGI
#siege -c25 -b -t15S -f tests/urls.txt
test_multi_cgi_stress() {
  log "Test 21: Multi-CGI Parallel Port Stress"
  run "Test 21: Multi-CGI mixed load" "siege -c25 -b -t15S -f tests/urls.txt"
}

##################
#   EXECUTION    #
##################
log "STARTING STRESS TESTS"

test_basic_get
test_sustained_load
test_homepage
test_long_load
test_static_file
test_404
test_error_page
test_keep_alive
test_autoindex
test_get_stress
test_empty_page

log "STARTING POST STRESS TESTS"

test_post_single
test_post_sustained
test_post_heavy
test_post_root
test_post_404
test_post_keep_alive

log "STARTING OTHER TESTS"
test_redirect_stress
test_delete_stress
test_post_big_files_stress
test_multi_cgi_stress

show_results
exit $EXIT_CODE
