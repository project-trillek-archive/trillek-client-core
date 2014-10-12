#ifndef TRANSFORM_SYSTEM_TEST_H_INCLUDED
#define TRANSFORM_SYSTEM_TEST_H_INCLUDED

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"
#include <string>
#include <exception>

#include "systems/transform-system.hpp"
#include "transform.hpp"

using namespace trillek;

namespace {
    TEST(TransformTest, GetTranslation) {
        trillek::Transform transform(0);
        glm::vec3 translation = transform.GetTranslation();

        EXPECT_FLOAT_EQ(translation.x, 0.0f);
        EXPECT_FLOAT_EQ(translation.y, 0.0f);
        EXPECT_FLOAT_EQ(translation.z, 0.0f);
    }
    TEST(TransformTest, Translate) {
        trillek::Transform transform(0);
        transform.Translate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 translation = transform.GetTranslation();

        EXPECT_FLOAT_EQ(translation.x, 1.0f);
        EXPECT_FLOAT_EQ(translation.y, 1.0f);
        EXPECT_FLOAT_EQ(translation.z, 1.0f);

        transform.Translate(glm::vec3(1.0f, 1.0f, 1.0f));
        translation = transform.GetTranslation();

        EXPECT_FLOAT_EQ(translation.x, 2.0f);
        EXPECT_FLOAT_EQ(translation.y, 2.0f);
        EXPECT_FLOAT_EQ(translation.z, 2.0f);
    }
    TEST(TransformTest, SetTranslation) {
        trillek::Transform transform(0);
        transform.Translate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 translation = transform.GetTranslation();

        EXPECT_FLOAT_EQ(translation.x, 1.0f);
        EXPECT_FLOAT_EQ(translation.y, 1.0f);
        EXPECT_FLOAT_EQ(translation.z, 1.0f);

        transform.SetTranslation(glm::vec3(5.0f, 5.0f, 5.0f));
        translation = transform.GetTranslation();

        EXPECT_FLOAT_EQ(translation.x, 5.0f);
        EXPECT_FLOAT_EQ(translation.y, 5.0f);
        EXPECT_FLOAT_EQ(translation.z, 5.0f);
    }

    TEST(TransformTest, GetRotation) {
        trillek::Transform transform(0);
        glm::vec3 rotation = transform.GetRotation();

        EXPECT_FLOAT_EQ(rotation.x, 0.0f);
        EXPECT_FLOAT_EQ(rotation.y, 0.0f);
        EXPECT_FLOAT_EQ(rotation.z, 0.0f);
    }
    TEST(TransformTest, Rotate) {
        trillek::Transform transform(0);
        transform.Rotate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 rotation = transform.GetRotation();

        EXPECT_FLOAT_EQ(rotation.x, 1.0f);
        EXPECT_FLOAT_EQ(rotation.y, 1.0f);
        EXPECT_FLOAT_EQ(rotation.z, 1.0f);

        transform.Rotate(glm::vec3(1.0f, 1.0f, 1.0f));
        rotation = transform.GetRotation();

        EXPECT_FLOAT_EQ(rotation.x, 2.0f);
        EXPECT_FLOAT_EQ(rotation.y, 2.0f);
        EXPECT_FLOAT_EQ(rotation.z, 2.0f);
    }
    TEST(TransformTest, SetRotation) {
        trillek::Transform transform(0);
        transform.Rotate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 rotation = transform.GetRotation();

        EXPECT_FLOAT_EQ(rotation.x, 1.0f);
        EXPECT_FLOAT_EQ(rotation.y, 1.0f);
        EXPECT_FLOAT_EQ(rotation.z, 1.0f);

        transform.SetRotation(glm::vec3(1.4f, 1.4f, 1.4f));
        rotation = transform.GetRotation();

        EXPECT_FLOAT_EQ(rotation.x, 1.4);
        EXPECT_FLOAT_EQ(rotation.y, 1.400001);
        EXPECT_FLOAT_EQ(rotation.z, 1.4);
    }

    TEST(TransformTest, GetScale) {
        trillek::Transform transform(0);
        glm::vec3 scale = transform.GetScale();

        EXPECT_FLOAT_EQ(scale.x, 1.0f);
        EXPECT_FLOAT_EQ(scale.y, 1.0f);
        EXPECT_FLOAT_EQ(scale.z, 1.0f);
    }
    TEST(TransformTest, Scale) {
        trillek::Transform transform(0);
        transform.Scale(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 scale = transform.GetScale();

        EXPECT_FLOAT_EQ(scale.x, 1.0f);
        EXPECT_FLOAT_EQ(scale.y, 1.0f);
        EXPECT_FLOAT_EQ(scale.z, 1.0f);

        transform.Scale(glm::vec3(2.0f, 2.0f, 2.0f));
        scale = transform.GetScale();

        EXPECT_FLOAT_EQ(scale.x, 2.0f);
        EXPECT_FLOAT_EQ(scale.y, 2.0f);
        EXPECT_FLOAT_EQ(scale.z, 2.0f);
    }
    TEST(TransformTest, SetScale) {
        trillek::Transform transform(0);
        transform.Scale(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 scale = transform.GetScale();

        EXPECT_FLOAT_EQ(scale.x, 1.0f);
        EXPECT_FLOAT_EQ(scale.y, 1.0f);
        EXPECT_FLOAT_EQ(scale.z, 1.0f);

        transform.SetScale(glm::vec3(5.0f, 5.0f, 5.0f));
        scale = transform.GetScale();

        EXPECT_FLOAT_EQ(scale.x, 5.0f);
        EXPECT_FLOAT_EQ(scale.y, 5.0f);
        EXPECT_FLOAT_EQ(scale.z, 5.0f);
    }

    TEST(TransformTest, GetOrientation) {
        trillek::Transform transform(0);
        glm::quat orientation = transform.GetOrientation();

        glm::quat qX = glm::angleAxis(0.0f, RIGHT_VECTOR);
        glm::quat qY = glm::angleAxis(0.0f, UP_VECTOR);
        glm::quat qZ = glm::angleAxis(0.0f, FORWARD_VECTOR);
        glm::quat final_orientation = qX * qY * qZ;
        final_orientation = glm::normalize(final_orientation);

        EXPECT_FLOAT_EQ(final_orientation.x, orientation.x);
        EXPECT_FLOAT_EQ(final_orientation.y, orientation.y);
        EXPECT_FLOAT_EQ(final_orientation.z, orientation.z);
        EXPECT_FLOAT_EQ(final_orientation.w, orientation.w);
    }
    TEST(TransformTest, SetOrientation) {
        trillek::Transform transform(0);
        transform.Rotate(glm::vec3(1.0f, 1.0f, 1.0f));
        glm::quat orientation = transform.GetOrientation();
        glm::quat final_orientation(glm::vec3(1.0f, 1.0f, 1.0f));

        EXPECT_FLOAT_EQ(final_orientation.x, orientation.x);
        EXPECT_FLOAT_EQ(final_orientation.y, orientation.y);
        EXPECT_FLOAT_EQ(final_orientation.z, orientation.z);
        EXPECT_FLOAT_EQ(final_orientation.w, orientation.w);

        transform.SetRotation(glm::vec3(glm::radians(5.0), glm::radians(5.0), glm::radians(5.0)));
        orientation = transform.GetOrientation();

        final_orientation = glm::quat(glm::vec3(glm::radians(5.0), glm::radians(5.0), glm::radians(5.0)));

        EXPECT_FLOAT_EQ(final_orientation.x, orientation.x);
        EXPECT_FLOAT_EQ(final_orientation.y, orientation.y);
        EXPECT_FLOAT_EQ(final_orientation.z, orientation.z);
        EXPECT_FLOAT_EQ(final_orientation.w, orientation.w);
    }
}  // namespace

#endif
