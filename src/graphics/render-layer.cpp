
#include "graphics/render-layer.hpp"

namespace trillek {
namespace graphics {

RenderAttachment::RenderAttachment() {
    renderbuf = 0;
    this->attachtarget = GL_COLOR_ATTACHMENT0;
    this->multisample = false;
    this->clearonuse = false;
    this->multisample_texture = false;
}

RenderAttachment::~RenderAttachment() {

}

RenderAttachment::RenderAttachment(RenderAttachment &&that) {
    this->renderbuf = that.renderbuf;
    this->texturename = std::move(that.texturename);
    this->attachtarget = that.attachtarget;
    this->multisample = that.multisample;
    this->multisample_texture = that.multisample_texture;
    this->clearonuse = that.clearonuse;
    that.renderbuf = 0;
    this->texture = std::move(that.texture);
}

RenderAttachment& RenderAttachment::operator=(RenderAttachment &&that) {
    this->renderbuf = that.renderbuf;
    this->texturename = std::move(that.texturename);
    this->attachtarget = that.attachtarget;
    this->multisample = that.multisample;
    this->multisample_texture = that.multisample_texture;
    this->clearonuse = that.clearonuse;
    that.renderbuf = 0;
    this->texture = std::move(that.texture);
    return *this;
}

bool RenderAttachment::SystemStart(const std::list<Property> &) {
    return false;
}
bool RenderAttachment::SystemReset(const std::list<Property> &) {
    return false;
}
bool RenderAttachment::Serialize(rapidjson::Document& document) {
    return false;
}

bool RenderAttachment::Parse(const std::string &object_name, rapidjson::Value& node) {
    return true;
}

RenderLayer::RenderLayer() {
    fbo_id = 0;
}

RenderLayer::~RenderLayer() {

}
bool RenderLayer::SystemStart(const std::list<Property> &) {
    return false;
}
bool RenderLayer::SystemReset(const std::list<Property> &) {
    return false;
}
bool RenderLayer::Serialize(rapidjson::Document& document) {
    return false;
}

void RenderLayer::BindToRender() const {}
void RenderLayer::UnbindFromAll() const {}
void RenderLayer::BindToRead() const {}

bool RenderLayer::Parse(const std::string &object_name, rapidjson::Value& node) {
    return true;
}

} // namespace graphics
} // namespace trillek
