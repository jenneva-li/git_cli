#include <gtest/gtest.h>  
#include <fstream>        
#include <filesystem>     
#include <string>          
#include <iostream>       

#include "repository.h"

namespace fs = std::filesystem;

class GitInitTest : public ::testing::Test {
protected:
    fs::path tempDir;

    void SetUp() override {
        tempDir = fs::temp_directory_path() / fs::path("git_test_repo");
        if (fs::exists(tempDir)) {
            fs::remove_all(tempDir);
        }
        fs::create_directory(tempDir);
    }

    void TearDown() override {
        if (fs::exists(tempDir)) {
            fs::remove_all(tempDir);
        }
    }
};

TEST_F(GitInitTest, CreatesGitDirectoryStructure) {
    auto repo = GitRepository::repo_create(tempDir);

    EXPECT_TRUE(fs::exists(repo.get_gitdir()));
    EXPECT_TRUE(fs::is_directory(repo.get_gitdir()));

    EXPECT_TRUE(fs::exists(repo.get_gitdir() / "objects"));
    EXPECT_TRUE(fs::exists(repo.get_gitdir() / "refs/heads"));
    EXPECT_TRUE(fs::exists(repo.get_gitdir() / "refs/tags"));
}

TEST_F(GitInitTest, CreatesDescriptionFile) {
    auto repo = GitRepository::repo_create(tempDir);
    auto descriptionFile = repo.get_gitdir() / "description";

    EXPECT_TRUE(fs::exists(descriptionFile));

    std::ifstream f(descriptionFile);
    std::string line;
    std::getline(f, line);
    EXPECT_NE(line.find("Unnamed repository"), std::string::npos);
}

TEST_F(GitInitTest, CreatesHEADFile) {
    auto repo = GitRepository::repo_create(tempDir);
    auto headFile = repo.get_gitdir() / "HEAD";

    EXPECT_TRUE(fs::exists(headFile));

    std::ifstream f(headFile);
    std::string line;
    std::getline(f, line);
    EXPECT_EQ(line, "ref: refs/heads/master");
}

TEST_F(GitInitTest, ThrowsIfDirectoryNotEmpty) {
    std::ofstream f(tempDir / "dummy.txt");
    f << "not empty";

    EXPECT_THROW({
        GitRepository::repo_create(tempDir);
    }, std::runtime_error);
}
