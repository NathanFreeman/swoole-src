#include "test_core.h"
#include "swoole_lru_cache.h"

using namespace swoole;
using namespace std;

int dtor_num = 0;
class lru_cache_test_class {
  public:
    lru_cache_test_class() {}

    ~lru_cache_test_class() {
        ++dtor_num;
    }
};

TEST(lru_cache, basic) {
    LRUCache<string> cache(2);
    shared_ptr<string> val = make_shared<string>("hello");
    shared_ptr<string> val1 = make_shared<string>("hello1");

    cache.set("test", val);
    ASSERT_EQ(cache.get("test").get(), val.get());

    cache.set("test", val1);
    ASSERT_EQ(cache.get("test").get(), val1.get());

    cache.del("test");
    ASSERT_EQ(cache.get("test"), nullptr);

    cache.set("test", val);
    ASSERT_EQ(cache.get("test").get(), val.get());

    cache.clear();
    ASSERT_EQ(cache.get("test"), nullptr);
}

TEST(lru_cache, memory_free) {
    LRUCache<lru_cache_test_class> cache(2);
    shared_ptr<lru_cache_test_class> val = make_shared<lru_cache_test_class>();
    cache.set("test", val);
    ASSERT_EQ(cache.get("test").get(), val.get());
    val.reset();
    ASSERT_EQ(dtor_num, 0);
    cache.clear();
    ASSERT_EQ(dtor_num, 1);
}

TEST(lru_cache, lru_kick) {
    LRUCache<lru_cache_test_class> cache(2);
    dtor_num = 0;
    shared_ptr<lru_cache_test_class> val = make_shared<lru_cache_test_class>();
    shared_ptr<lru_cache_test_class> val1 = make_shared<lru_cache_test_class>();
    shared_ptr<lru_cache_test_class> val2 = make_shared<lru_cache_test_class>();
    shared_ptr<lru_cache_test_class> val3 = make_shared<lru_cache_test_class>();

    cache.set("test", val);
    ASSERT_EQ(cache.get("test").get(), val.get());
    val.reset();
    ASSERT_EQ(dtor_num, 0);

    cache.set("test1", val1);
    ASSERT_EQ(cache.get("test1").get(), val1.get());
    val1.reset();
    ASSERT_EQ(dtor_num, 0);

    cache.set("test2", val2);
    ASSERT_EQ(cache.get("test2").get(), val2.get());
    val2.reset();
    ASSERT_EQ(dtor_num, 1);
    ASSERT_EQ(cache.get("test"), nullptr);

    shared_ptr<lru_cache_test_class> val4 = make_shared<lru_cache_test_class>();
    cache.set("test1", val4);  // update test1 and will del test2
    ASSERT_EQ(cache.get("test1").get(), val4.get());
    ASSERT_EQ(dtor_num, 2);

    cache.set("test3", val3);
    ASSERT_EQ(cache.get("test3").get(), val3.get());
    val3.reset();
    ASSERT_EQ(dtor_num, 3);
    ASSERT_EQ(cache.get("test2"), nullptr);

    cache.clear();
    ASSERT_EQ(dtor_num, 4);
}
