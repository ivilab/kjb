
/* $Id: test.h 6414 2010-07-29 18:43:32Z ksimek $ */

#ifndef TEST_H
#define TEST_H

#define DB(x) std::cout << #x << ": \n" << x << std::endl;

#define TEST_TRUE(line) \
    if(!(line)) \
    { \
        printf("Test failed: \"%s\" (%s:%d)\n", #line, __FILE__, __LINE__); \
        abort(); \
    }

#define TEST_FALSE(line) \
    if(line) \
    { \
        printf("Test failed: \"%s\" (%s:%d)\n", #line, __FILE__, __LINE__); \
        abort(); \
    }

#define TEST_SUCCESS(line) \
    try{line;} \
    catch(...) \
    { \
        printf("Test-Success failed: \"%s\" (%s:%d)\n", #line, __FILE__, __LINE__); \
        abort(); \
    }

#define TEST_FAIL(line) \
    { \
        bool assert_fail_failed = false; \
        try{line;} \
        catch(...) \
        { \
            assert_fail_failed = true; \
        } \
        if(!assert_fail_failed) \
        { \
            printf("Test-fail failed: \"%s\" (%s:%d)\n", #line, __FILE__, __LINE__); \
            abort(); \
        } \
    }

#define VEC_EQUAL(a,b) ((a - b).magnitude() <= FLT_EPSILON)
#define TEST_VEC_EQUAL(a,b) \
    { \
        if(!VEC_EQUAL(a,b)) \
        { \
            printf("TEST_VEC_EQUAL failed \"%s, %s\" (%s:%d)\n", #a, #b, __FILE__, __LINE__); \
            abort(); \
        } \
    }

#define MAT_EQUAL(a,b) ((a - b).abs_of_determinant() <= FLT_EPSILON)

#define TEST_MAT_EQUAL(a,b) \
    { \
        if(!MAT_EQUAL(a,b)) \
        { \
            printf("TEST_MAT_EQUAL failed \"%s, %s\" (%s:%d)\n", #a, #b, __FILE__, __LINE__); \
            abort(); \
        } \
    }

#define FLT_EQUAL(a,b) (fabs(a - b) <= FLT_EPSILON)
#define TEST_FLT_EQUAL(a,b) \
    { \
        if(!FLT_EQUAL(a,b)) \
        { \
            printf("TEST_FLT_EQUAL failed \"%s, %s\" (%s:%d)\n", #a, #b, __FILE__, __LINE__); \
            abort(); \
        } \
    }

#endif
