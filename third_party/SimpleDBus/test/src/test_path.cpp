#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <simpledbus/base/Path.h>

using namespace SimpleDBus;

TEST(Path, CountElements) {
    EXPECT_EQ(0, Path::count_elements("/"));
    EXPECT_EQ(1, Path::count_elements("/a"));
    EXPECT_EQ(2, Path::count_elements("/a/b"));
    EXPECT_EQ(3, Path::count_elements("/a/b/c"));
}

TEST(Path, FetchElements) {
    EXPECT_EQ("/", Path::fetch_elements("/a/b/c/d", 0));
    EXPECT_EQ("/a", Path::fetch_elements("/a/b/c/d", 1));
    EXPECT_EQ("/a/b", Path::fetch_elements("/a/b/c/d", 2));
    EXPECT_EQ("/a/b/c", Path::fetch_elements("/a/b/c/d", 3));
    EXPECT_EQ("/a/b/c/d", Path::fetch_elements("/a/b/c/d", 4));
}

TEST(Path, SplitElements) {
    EXPECT_THAT(Path::split_elements("/"), ::testing::SizeIs(0));
    EXPECT_THAT(Path::split_elements("/a"), ::testing::ElementsAre("a"));
    EXPECT_THAT(Path::split_elements("/a/b"), ::testing::ElementsAre("a", "b"));
    EXPECT_THAT(Path::split_elements("/a/b/c"), ::testing::ElementsAre("a", "b", "c"));
    EXPECT_THAT(Path::split_elements("/a/b/c/d"), ::testing::ElementsAre("a", "b", "c", "d"));
}

TEST(Path, RecognizeDescendant) {
    EXPECT_FALSE(Path::is_descendant("/a/b", "/a/b"));
    EXPECT_FALSE(Path::is_descendant("/a/b", "/a"));
    EXPECT_FALSE(Path::is_descendant("/a/b", "/"));

    EXPECT_TRUE(Path::is_descendant("/a/b", "/a/b/c"));
    EXPECT_TRUE(Path::is_descendant("/a/b", "/a/b/c/d"));
    EXPECT_TRUE(Path::is_descendant("/a/b", "/a/b/c/d/e"));
}

TEST(Path, RecognizeAscendant) {
    EXPECT_FALSE(Path::is_ascendant("/a/b", "/a/b"));
    EXPECT_TRUE(Path::is_ascendant("/a/b", "/a"));
    EXPECT_TRUE(Path::is_ascendant("/a/b", "/"));

    EXPECT_FALSE(Path::is_ascendant("/a/b", "/a/b/c"));
    EXPECT_FALSE(Path::is_ascendant("/a/b", "/a/b/c/d"));
    EXPECT_FALSE(Path::is_ascendant("/a/b", "/a/b/c/d/e"));
}

TEST(Path, RecognizeChild) {
    EXPECT_FALSE(Path::is_child("/a/b", "/a/b"));
    EXPECT_FALSE(Path::is_child("/a/b", "/a"));
    EXPECT_FALSE(Path::is_child("/a/b", "/"));

    EXPECT_TRUE(Path::is_child("/a/b", "/a/b/c"));
    EXPECT_FALSE(Path::is_child("/a/b", "/a/b/c/d"));
    EXPECT_FALSE(Path::is_child("/a/b", "/a/b/c/d/e"));
}

TEST(Path, RecognizeParent) {
    EXPECT_FALSE(Path::is_parent("/a/b", "/a/b"));
    EXPECT_TRUE(Path::is_parent("/a/b", "/a"));
    EXPECT_FALSE(Path::is_parent("/a/b", "/"));

    EXPECT_FALSE(Path::is_parent("/a/b", "/a/b/c"));
    EXPECT_FALSE(Path::is_parent("/a/b", "/a/b/c/d"));
    EXPECT_FALSE(Path::is_parent("/a/b", "/a/b/c/d/e"));
}

TEST(Path, GenerateNextChild) {
    EXPECT_EQ("/a", Path::next_child("/", "/a/b/c/d/e"));
    EXPECT_EQ("/a/b", Path::next_child("/a", "/a/b/c/d/e"));
    EXPECT_EQ("/a/b/c", Path::next_child("/a/b", "/a/b/c/d/e"));
    EXPECT_EQ("/a/b/c/d", Path::next_child("/a/b/c", "/a/b/c/d/e"));
}
