#include <gtest/gtest.h>
#include <typelib/typelib.h>
#include <typelib/compat.h>

typedef struct {
    tl_DLISTNODE list;
    const char *desc;
} todo_t;

class List : public ::testing::Test
{
};

TEST_F(List, basicTests)
{
    tl_DLIST root;
    memset(&root, 0, sizeof(todo_t));
    tl_dlist_init(&root);
    ASSERT_EQ(&root.base, root.base.next);
    ASSERT_EQ(&root.base, root.base.prev);

    todo_t t0 = {{NULL, NULL}, "break"};
    tl_dlist_append(&root, &t0.list);
    ASSERT_EQ(&t0.list, root.base.next);
    ASSERT_EQ(&t0.list, root.base.prev);
    ASSERT_EQ(1, root.size);

    tl_dlist_delete(&root, &t0.list);
    ASSERT_EQ(NULL, t0.list.next);
    ASSERT_EQ(NULL, t0.list.prev);
    ASSERT_EQ(&root.base, root.base.next);
    ASSERT_EQ(&root.base, root.base.prev);

    todo_t t1 = {{NULL, NULL}, "write"};
    ASSERT_STREQ("write", t1.desc);
    tl_dlist_append(&root, &t1.list);
    ASSERT_EQ(&t1.list, root.base.next);
    ASSERT_EQ(&t1.list, root.base.prev);

    todo_t t2 = {{NULL, NULL}, "test"};
    ASSERT_STREQ("test", t2.desc);
    tl_dlist_append(&root, &t2.list);
    ASSERT_EQ(&t1.list, root.base.next);
    ASSERT_EQ(&t2.list, root.base.prev);

    todo_t t3 = {{NULL, NULL}, "refactor"};
    ASSERT_STREQ("refactor", t3.desc);
    tl_dlist_append(&root, &t3.list);
    ASSERT_EQ(&t1.list, root.base.next);
    ASSERT_EQ(&t3.list, root.base.prev);

    todo_t t4 = {{NULL, NULL}, "read"};
    ASSERT_STREQ("read", t4.desc);
    tl_dlist_prepend(&root, &t4.list);
    ASSERT_EQ(&t4.list, root.base.next);
    ASSERT_EQ(&t3.list, root.base.prev);

    tl_DLISTNODE *ii = root.base.next;
    todo_t *tt;

    tt = TL_DLIST_ITEM(ii, todo_t, list);
    ASSERT_STREQ("read", tt->desc);
    ii = ii->next;
    tt = TL_DLIST_ITEM(ii, todo_t, list);
    ASSERT_STREQ("write", tt->desc);
    ii = ii->next;
    tt = TL_DLIST_ITEM(ii, todo_t, list);
    ASSERT_STREQ("test", tt->desc);
    ii = ii->next;
    tt = TL_DLIST_ITEM(ii, todo_t, list);
    ASSERT_STREQ("refactor", tt->desc);

    tl_DLISTNODE *nn;
    TL_DLIST_SAFE_FOR(ii, nn, &root) {
        tt = TL_DLIST_ITEM(ii, todo_t, list);
        tl_dlist_delete(&root, &tt->list);
        memset(tt, 0, sizeof(todo_t));
    }
    ASSERT_EQ(&root.base, root.base.next);
    ASSERT_EQ(&root.base, root.base.prev);
}

typedef struct {
    tl_DLISTNODE list;
    int number;
} num_t;

int ascending(tl_DLISTNODE *a, tl_DLISTNODE *b)
{
    num_t *aa, *bb;

    aa = TL_DLIST_ITEM(a, num_t, list);
    bb = TL_DLIST_ITEM(b, num_t, list);
    if (aa->number > bb->number) {
        return 1;
    } else if (aa->number < bb->number) {
        return -1;
    } else {
        return 0;
    }
}

TEST_F(List, sortedListTest)
{
    tl_DLIST root;
    memset(&root, 0, sizeof(num_t));
    tl_dlist_init(&root);

    num_t n0 = {{NULL, NULL}, 0};
    tl_dlist_add_sorted(&root, &n0.list, ascending);
    num_t n3 = {{NULL, NULL}, 3};
    tl_dlist_add_sorted(&root, &n3.list, ascending);
    num_t n2 = {{NULL, NULL}, 2};
    tl_dlist_add_sorted(&root, &n2.list, ascending);
    num_t n7 = {{NULL, NULL}, 7};
    tl_dlist_add_sorted(&root, &n7.list, ascending);
    num_t n1 = {{NULL, NULL}, 1};
    tl_dlist_add_sorted(&root, &n1.list, ascending);

    tl_DLISTNODE *ii = root.base.next;
    num_t *nn;
    nn = TL_DLIST_ITEM(ii, num_t, list);
    ASSERT_EQ(0, nn->number);
    ii = ii->next;
    nn = TL_DLIST_ITEM(ii, num_t, list);
    ASSERT_EQ(1, nn->number);
    ii = ii->next;
    nn = TL_DLIST_ITEM(ii, num_t, list);
    ASSERT_EQ(2, nn->number);
    ii = ii->next;
    nn = TL_DLIST_ITEM(ii, num_t, list);
    ASSERT_EQ(3, nn->number);
    ii = ii->next;
    nn = TL_DLIST_ITEM(ii, num_t, list);
    ASSERT_EQ(7, nn->number);
    ii = ii->next;
}
