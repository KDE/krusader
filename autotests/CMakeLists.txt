add_test(NAME krusader-self-test COMMAND krusader --self-test)
set_tests_properties(krusader-self-test PROPERTIES TIMEOUT 10)
