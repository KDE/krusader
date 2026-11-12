add_test(NAME krusader-self-test COMMAND krusader --self-test)
# Note: 10 seconds were not enough on 2026-11-10. Use 30 seconds, as in Kate and Dolphin
set_tests_properties(krusader-self-test PROPERTIES TIMEOUT 30)
