/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2012 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <typelib/compat.h>
#include <gtest/gtest.h>
#include <string>

class String : public ::testing::Test
{
};

TEST_F(String, testBasic)
{
    int rv;
    lcb_string str;
    rv = lcb_string_init(&str);
    ASSERT_EQ(0, rv);
    ASSERT_EQ(NULL, str.base);
    ASSERT_EQ(0, str.nalloc);
    ASSERT_EQ(0, str.nused);

    rv = lcb_string_append(&str, "Hello", 5);
    ASSERT_EQ(0, rv);

    rv = lcb_string_append(&str, "blah", -1);
    ASSERT_EQ(-1, rv);

    rv = lcb_string_appendz(&str, "blah");
    ASSERT_EQ(0, rv);

    ASSERT_EQ(0, strcmp("Helloblah", str.base));

    lcb_string_erase_beginning(&str, 5);
    ASSERT_EQ(0, strcmp("blah", str.base));

    lcb_string_erase_end(&str, 4);
    ASSERT_EQ(0, strcmp("", str.base));

    lcb_string_clear(&str);
    ASSERT_TRUE(str.base != NULL);

    lcb_string_release(&str);
    ASSERT_EQ(NULL, str.base);
}

TEST_F(String, testAdvance)
{
    int rv;
    lcb_string str;
    rv = lcb_string_init(&str);
    ASSERT_EQ(0, rv);

    rv = lcb_string_reserve(&str, 30);
    ASSERT_EQ(0, rv);
    ASSERT_TRUE(str.nalloc >= 30);
    ASSERT_EQ(0, str.nused);

    memcpy(lcb_string_tail(&str), "Hello", 5);
    lcb_string_added(&str, 5);
    ASSERT_EQ(5, str.nused);
    ASSERT_EQ(0, strcmp(str.base, "Hello"));
    lcb_string_release(&str);
}

TEST_F(String, testFormat)
{
    int rv;
    tl_STRING str;
    ASSERT_EQ(0, tl_str_init(&str));
    std::string s(65536, '*');
    rv = tl_str_appendf(&str, "%s-%s-%s", s.c_str(), s.c_str(), s.c_str());
    ASSERT_EQ(0, rv);
    tl_str_cleanup(&str);
}
