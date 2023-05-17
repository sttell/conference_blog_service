WITH_CACHE="ON"

STRESS_TEST_REQUEST_SCRIPT="get_no_cache.lua"
if [[ "${WITH_CACHE}" == "ON" ]]; then
  STRESS_TEST_REQUEST_SCRIPT="get_cached.lua"
fi

echo "========== Test with 1 threads ================"
wrk -d 10 -t 1 -c 1 --latency -s ${STRESS_TEST_REQUEST_SCRIPT} http://localhost:8080/

echo "========== Test with 2 threads ================"
wrk -d 10 -t 2 -c 2 --latency -s ${STRESS_TEST_REQUEST_SCRIPT} http://localhost:8080/

echo "========== Test with 5 threads ================"
wrk -d 10 -t 5 -c 5 --latency -s ${STRESS_TEST_REQUEST_SCRIPT} http://localhost:8080/

echo "========== Test with 10 threads ================"
wrk -d 10 -t 10 -c 10 --latency -s ${STRESS_TEST_REQUEST_SCRIPT} http://localhost:8080/



