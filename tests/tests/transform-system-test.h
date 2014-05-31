#ifndef TRANSFORM_SYSTEM_TEST_H_INCLUDED
#define TRANSFORM_SYSTEM_TEST_H_INCLUDED

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"
#include <string>

#include "systems/transform-system.hpp"
#include "resources/transform.hpp"

using namespace trillek;

namespace {
    TEST(TransformSystemTest, AddTransform) {
        // Get the instance allocated, and then we can use the shortcut static mathods.
        transform::System::GetInstance();

        auto transform = transform::System::AddTransform(0);

        EXPECT_TRUE(transform != nullptr);
    }
    TEST(TransformSystemTest, GetTransform) {
        auto transform = transform::System::GetTransform(0);

        EXPECT_TRUE(transform != nullptr);
    }
    TEST(TransformSystemTest, AddExistingTransform) {
        auto transform = transform::System::GetTransform(0);
        auto transform2 = transform::System::AddTransform(0);

        EXPECT_TRUE(transform == transform2);
    }
    TEST(TransformSystemTest, RemoveTransform) {
        transform::System::RemoveTransform(0);

        auto transform = transform::System::GetTransform(0);

        EXPECT_TRUE(transform == nullptr);
    }

    TEST(TransformTest, GetTranslation) {
        trillek::transform::Transform transform;
        glm::vec3 translation = transform.GetTranslation();

        EXPECT_TRUE(glm::vec3(0.0f, 0.0f, 0.0f) == translation);
    }
    TEST(TransformTest, Translate) {
        trillek::transform::Transform transform;
        transform.Translate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 translation = transform.GetTranslation();

        EXPECT_TRUE(glm::vec3(1.0f, 1.0f, 1.0f) == translation);

        transform.Translate(glm::vec3(1.0f, 1.0f, 1.0f));
        translation = transform.GetTranslation();

        EXPECT_TRUE(glm::vec3(2.0f, 2.0f, 2.0f) == translation);
    }
    TEST(TransformTest, SetTranslation) {
        trillek::transform::Transform transform;
        transform.Translate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 translation = transform.GetTranslation();

        EXPECT_TRUE(glm::vec3(1.0f, 1.0f, 1.0f) == translation);

        transform.SetTranslation(glm::vec3(5.0f, 5.0f, 5.0f));
        translation = transform.GetTranslation();

        EXPECT_TRUE(glm::vec3(5.0f, 5.0f, 5.0f) == translation);
    }

    TEST(TransformTest, GetRotation) {
        trillek::transform::Transform transform;
        glm::vec3 rotation = transform.GetRotation();

        EXPECT_TRUE(glm::vec3(0.0f, 0.0f, 0.0f) == rotation);
    }
    TEST(TransformTest, Rotate) {
        trillek::transform::Transform transform;
        transform.Rotate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 rotation = transform.GetRotation();

        EXPECT_TRUE(glm::vec3(1.0f, 1.0f, 1.0f) == rotation);

        transform.Rotate(glm::vec3(1.0f, 1.0f, 1.0f));
        rotation = transform.GetRotation();

        EXPECT_TRUE(glm::vec3(2.0f, 2.0f, 2.0f) == rotation);
    }
    TEST(TransformTest, SetRotation) {
        trillek::transform::Transform transform;
        transform.Rotate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 rotation = transform.GetRotation();

        EXPECT_TRUE(glm::vec3(1.0f, 1.0f, 1.0f) == rotation);

        transform.SetRotation(glm::vec3(5.0f, 5.0f, 5.0f));
        rotation = transform.GetRotation();

        EXPECT_TRUE(glm::vec3(5.0f, 5.0f, 5.0f) == rotation);
    }

    TEST(TransformTest, GetScale) {
        trillek::transform::Transform transform;
        glm::vec3 scale = transform.GetScale();

        EXPECT_TRUE(glm::vec3(0.0f, 0.0f, 0.0f) == scale);
    }
    TEST(TransformTest, Scale) {
        trillek::transform::Transform transform;
        transform.Scale(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 scale = transform.GetScale();

        EXPECT_TRUE(glm::vec3(1.0f, 1.0f, 1.0f) == scale);

        transform.Scale(glm::vec3(1.0f, 1.0f, 1.0f));
        scale = transform.GetScale();

        EXPECT_TRUE(glm::vec3(2.0f, 2.0f, 2.0f) == scale);
    }
    TEST(TransformTest, SetScale) {
        trillek::transform::Transform transform;
        transform.Scale(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 scale = transform.GetScale();

        EXPECT_TRUE(glm::vec3(1.0f, 1.0f, 1.0f) == scale);

        transform.SetScale(glm::vec3(5.0f, 5.0f, 5.0f));
        scale = transform.GetScale();

        EXPECT_TRUE(glm::vec3(5.0f, 5.0f, 5.0f) == scale);
    }

    TEST(TransformTest, GetOrientation) {
        trillek::transform::Transform transform;
        glm::quat orientation = transform.GetOrientation();

        glm::quat qX = glm::angleAxis(0.0f, transform::RIGHT_VECTOR);
        glm::quat qY = glm::angleAxis(0.0f, transform::UP_VECTOR);
        glm::quat qZ = glm::angleAxis(0.0f, transform::FORWARD_VECTOR);
        glm::quat final_orientation = qX * qY * qZ;
        final_orientation = glm::normalize(final_orientation);

        EXPECT_TRUE(final_orientation == orientation);
    }
    TEST(TransformTest, Orient) {
        trillek::transform::Transform transform;
        transform.Rotate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::quat orientation = transform.GetOrientation();

        glm::quat qX = glm::angleAxis(1.0f, transform::RIGHT_VECTOR);
        glm::quat qY = glm::angleAxis(1.0f, transform::UP_VECTOR);
        glm::quat qZ = glm::angleAxis(1.0f, transform::FORWARD_VECTOR);
        glm::quat final_orientation = qX * qY * qZ;
        final_orientation = glm::normalize(final_orientation);

        EXPECT_TRUE(final_orientation == orientation);
        final_orientation = glm::normalize(final_orientation *  final_orientation);

        transform.Rotate(glm::vec3(1.0f, 1.0f, 1.0f));
        orientation = transform.GetOrientation();

        //EXPECT_TRUE(final_orientation == orientation);
        EXPECT_FLOAT_EQ(final_orientation.x, orientation.x);
        EXPECT_FLOAT_EQ(final_orientation.y, orientation.y);
        EXPECT_FLOAT_EQ(final_orientation.z, orientation.z);
        EXPECT_FLOAT_EQ(final_orientation.w, orientation.w);
    }
    TEST(TransformTest, SetOrientation) {
        trillek::transform::Transform transform;
        transform.Rotate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::quat orientation = transform.GetOrientation();

        glm::quat qX = glm::angleAxis(1.0f, transform::RIGHT_VECTOR);
        glm::quat qY = glm::angleAxis(1.0f, transform::UP_VECTOR);
        glm::quat qZ = glm::angleAxis(1.0f, transform::FORWARD_VECTOR);
        glm::quat final_orientation = qX * qY * qZ;
        final_orientation = glm::normalize(final_orientation);

        EXPECT_TRUE(final_orientation == orientation);

        transform.SetRotation(glm::vec3(5.0f, 5.0f, 5.0f));
        orientation = transform.GetOrientation();

        qX = glm::angleAxis(5.0f, transform::RIGHT_VECTOR);
        qY = glm::angleAxis(5.0f, transform::UP_VECTOR);
        qZ = glm::angleAxis(5.0f, transform::FORWARD_VECTOR);
        final_orientation = qX * qY * qZ;
        final_orientation = glm::normalize(final_orientation);

        EXPECT_TRUE(final_orientation == orientation);
    }
}  // namespace

#endif
