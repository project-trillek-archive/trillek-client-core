
#include "graphics/render-layer.hpp"

namespace trillek {
namespace graphics {

RenderAttachment::RenderAttachment() {
    renderbuf = 0;
    this->attachtarget = GL_COLOR_ATTACHMENT0;
    this->multisample = false;
}

RenderAttachment::~RenderAttachment() {

}

RenderAttachment::RenderAttachment(RenderAttachment &&that) {
    this->renderbuf = that.renderbuf;
    this->texturename = std::move(that.texturename);
    this->attachtarget = that.attachtarget;
    this->multisample = that.multisample;
    that.renderbuf = 0;
    this->texture = std::move(that.texture);
}

RenderAttachment& RenderAttachment::operator=(RenderAttachment &&that) {
    this->renderbuf = that.renderbuf;
    this->texturename = std::move(that.texturename);
    this->attachtarget = that.attachtarget;
    this->multisample = that.multisample;
    that.renderbuf = 0;
    this->texture = std::move(that.texture);
    return *this;
}

bool RenderAttachment::Parse(rapidjson::Value& node) {
    return true;
}

RenderLayer::RenderLayer() {
    fbo_id = 0;
}

RenderLayer::~RenderLayer() {

}

bool RenderLayer::Parse(rapidjson::Value& node) {
    return true;
}

} // namespace graphics
} // namespace trillek
