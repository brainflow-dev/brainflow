#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <simpledbus/base/Path.h>

using namespace SimpleDBus;

TEST(Path, CountElements) {
    EXPECT_EQ(0, PathUtils::count_elements("/"));
    EXPECT_EQ(1, PathUtils::count_elements("/a"));
    EXPECT_EQ(2, PathUtils::count_elements("/a/b"));
    EXPECT_EQ(3, PathUtils::count_elements("/a/b/c"));
}

TEST(Path, FetchElements) {
    EXPECT_EQ("/", PathUtils::fetch_elements("/a/b/c/d", 0));
    EXPECT_EQ("/a", PathUtils::fetch_elements("/a/b/c/d", 1));
    EXPECT_EQ("/a/b", PathUtils::fetch_elements("/a/b/c/d", 2));
    EXPECT_EQ("/a/b/c", PathUtils::fetch_elements("/a/b/c/d", 3));
    EXPECT_EQ("/a/b/c/d", PathUtils::fetch_elements("/a/b/c/d", 4));
}

TEST(Path, SplitElements) {
    EXPECT_THAT(PathUtils::split_elements("/"), ::testing::SizeIs(0));
    EXPECT_THAT(PathUtils::split_elements("/a"), ::testing::ElementsAre("a"));
    EXPECT_THAT(PathUtils::split_elements("/a/b"), ::testing::ElementsAre("a", "b"));
    EXPECT_THAT(PathUtils::split_elements("/a/b/c"), ::testing::ElementsAre("a", "b", "c"));
    EXPECT_THAT(PathUtils::split_elements("/a/b/c/d"), ::testing::ElementsAre("a", "b", "c", "d"));
}

TEST(Path, RecognizeDescendant) {
    EXPECT_FALSE(PathUtils::is_descendant("/a/b", "/a/b"));
    EXPECT_FALSE(PathUtils::is_descendant("/a/b", "/a"));
    EXPECT_FALSE(PathUtils::is_descendant("/a/b", "/"));

    EXPECT_TRUE(PathUtils::is_descendant("/a/b", "/a/b/c"));
    EXPECT_TRUE(PathUtils::is_descendant("/a/b", "/a/b/c/d"));
    EXPECT_TRUE(PathUtils::is_descendant("/a/b", "/a/b/c/d/e"));
}

TEST(Path, RecognizeAscendant) {
    EXPECT_FALSE(PathUtils::is_ascendant("/a/b", "/a/b"));
    EXPECT_TRUE(PathUtils::is_ascendant("/a/b", "/a"));
    EXPECT_TRUE(PathUtils::is_ascendant("/a/b", "/"));

    EXPECT_FALSE(PathUtils::is_ascendant("/a/b", "/a/b/c"));
    EXPECT_FALSE(PathUtils::is_ascendant("/a/b", "/a/b/c/d"));
    EXPECT_FALSE(PathUtils::is_ascendant("/a/b", "/a/b/c/d/e"));
}

TEST(Path, RecognizeChild) {
    EXPECT_FALSE(PathUtils::is_child("/a/b", "/a/b"));
    EXPECT_FALSE(PathUtils::is_child("/a/b", "/a"));
    EXPECT_FALSE(PathUtils::is_child("/a/b", "/"));

    EXPECT_TRUE(PathUtils::is_child("/a/b", "/a/b/c"));
    EXPECT_FALSE(PathUtils::is_child("/a/b", "/a/b/c/d"));
    EXPECT_FALSE(PathUtils::is_child("/a/b", "/a/b/c/d/e"));
}

TEST(Path, RecognizeParent) {
    EXPECT_FALSE(PathUtils::is_parent("/a/b", "/a/b"));
    EXPECT_TRUE(PathUtils::is_parent("/a/b", "/a"));
    EXPECT_FALSE(PathUtils::is_parent("/a/b", "/"));

    EXPECT_FALSE(PathUtils::is_parent("/a/b", "/a/b/c"));
    EXPECT_FALSE(PathUtils::is_parent("/a/b", "/a/b/c/d"));
    EXPECT_FALSE(PathUtils::is_parent("/a/b", "/a/b/c/d/e"));
}

TEST(Path, GenerateNextChild) {
    EXPECT_EQ("/a", PathUtils::next_child("/", "/a/b/c/d/e"));
    EXPECT_EQ("/a/b", PathUtils::next_child("/a", "/a/b/c/d/e"));
    EXPECT_EQ("/a/b/c", PathUtils::next_child("/a/b", "/a/b/c/d/e"));
    EXPECT_EQ("/a/b/c/d", PathUtils::next_child("/a/b/c", "/a/b/c/d/e"));
}
