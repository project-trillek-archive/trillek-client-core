#ifndef RESOURCESYSTEM_TEST_H_INCLUDED
#define RESOURCESYSTEM_TEST_H_INCLUDED

#include "gtest/gtest.h"

#include "systems/resource-system.hpp"
#include "resources/text-file.hpp"

using namespace trillek::resource;

namespace trillek {
    // Check if resource types are registered correctly
    TEST(ResSysTest, Register) {
        ResourceMap::GetInstance();

        // If we have a valid type id retrieved from within resource system, then registtration was valid.
        ASSERT_EQ(reflection::GetTypeID<TextFile>(), ResourceMap::GetTypeIDFromName(reflection::GetTypeName<TextFile>()));
    }

    // Create a resource at compile time when type information is known.
    TEST(ResSysTest, CreateCompileTime) {
        std::vector<Property> props;
        Property p("filename", std::string("assets/tests/test.txt"));
        props.push_back(p);

        std::shared_ptr<TextFile> file = ResourceMap::Create<TextFile>("test", props);

        // This should be true as the resource was created properly.
        ASSERT_TRUE(file != nullptr);
    }

    TEST(ResSysTest, Exists) {
        ASSERT_TRUE(ResourceMap::Exists("test"));
    }

    TEST(ResSysTest, Remove) {
        ResourceMap::Remove("test");
        ASSERT_FALSE(ResourceMap::Exists("test"));
    }

    // Create a resource at runtime when type information is not known.
    TEST(ResSysTest, CreateRunTime) {
        std::vector<Property> props;
        Property p("filename", std::string("assets/tests/test.txt"));
        props.push_back(p);

        // If we have a valid type id retrieved from within resource system, then registtration was valid.
        // We are using the compile time ID in this instance incase it is changed in source. Normally this
        // woulnd't be used and, instead, would be obtained from the script or other loader.
        ASSERT_TRUE(ResourceMap::Create(reflection::GetTypeName<TextFile>(), "test", props));

        // We must make sure to remove the resource each time as resource system is a singleton and the
        // resource will persist.
        ResourceMap::Remove("test");
    }

    // Create a resource that doesn't exist.
    TEST(ResSysTest, CreateNonExistent) {
        std::vector<Property> props;
        Property p("filename", std::string("bad_test.txt"));
        props.push_back(p);

        std::shared_ptr<TextFile> file = ResourceMap::Create<TextFile>("test", props);

        // This should be true as the resource wasn't created properly.
        ASSERT_TRUE(file == nullptr);

        // We must make sure to remove the resource each time as resource system is a singleton and the
        // resource will persist.
        ResourceMap::Remove("test");
    }

    // Create a resource at runtime when type information is not known.
    TEST(ResSysTest, CreateInvalidType) {
        std::vector<Property> props;
        Property p("filename", std::string("assets/tests/test.txt"));
        props.push_back(p);

        // If we have a valid type id retrieved from within resource system, then registtration was valid.
        // We are using the compile time ID in this instance incase it is changed in source. Normally this
        // woulnd't be used and, instead, would be obtained from the script or other loader.
        ASSERT_FALSE(ResourceMap::Create("", "test", props));

        // We must make sure to remove the resource each time as resource system is a singleton and the
        // resource will persist.
        ResourceMap::Remove("test");
    }

    // Attempt to create a resource that already has been created. It should return the already created resource.
    TEST(ResSysTest, CreateAlreadyCreated) {
        std::vector<Property> props;
        Property p("filename", std::string("assets/tests/test.txt"));
        props.push_back(p);

        std::shared_ptr<TextFile> file = ResourceMap::Create<TextFile>("test", props);

        std::shared_ptr<TextFile> file2 = ResourceMap::Create<TextFile>("test", props);

        ASSERT_EQ(file.get(), file2.get());

        // We must make sure to remove the resource each time as resource system is a singleton and the
        // resource will persist.
        ResourceMap::Remove("test");
    }

    // Add a resource create in memory. Also checks if it keeps a strong reference.
    TEST(ResSysTest, AddFromMemory) {
        std::shared_ptr<TextFile> file(new TextFile());

        std::vector<Property> props;
        Property p("filename", std::string("assets/tests/test.txt"));
        props.push_back(p);

        file->Initialize(props);

        ResourceMap::Add<TextFile>("test", file);

        ASSERT_TRUE(ResourceMap::Exists("test"));

        // Check to make sure it still exists after the local strong reference is gone.
        file.reset();

        ASSERT_TRUE(ResourceMap::Exists("test"));

        // We must make sure to remove the resource each time as resource system is a singleton and the
        // resource will persist.
        ResourceMap::Remove("test");
    }

    // Add a resource create in memory. Also checks if it keeps a strong reference.
    TEST(ResSysTest, ModifyInMemory) {
        std::shared_ptr<TextFile> file(new TextFile());

        std::vector<Property> props;
        Property p("filename", std::string("assets/tests/test.txt"));
        props.push_back(p);

        file->Initialize(props);

        ResourceMap::Add<TextFile>("test", file);

        std::shared_ptr<TextFile> file2 = ResourceMap::Get<TextFile>("test");

        file->AppendText("?");

        ASSERT_TRUE(file->GetText() == file2->GetText());

        // We must make sure to remove the resource each time as resource system is a singleton and the
        // resource will persist.
        ResourceMap::Remove("test");
    }
}

#endif
