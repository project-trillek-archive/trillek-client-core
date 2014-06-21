
#include "graphics/render-layer.hpp"
#include "trillek-game.hpp"

namespace trillek {
namespace graphics {

RenderAttachment::RenderAttachment() {
    renderbuf = 0;
    this->attachtarget = GL_COLOR_ATTACHMENT0;
    this->multisample = false;
    this->clearonuse = false;
    this->multisample_texture = false;
    this->outputnumber = 0;
}

RenderAttachment::~RenderAttachment() {
    Destroy();
}

RenderAttachment::RenderAttachment(RenderAttachment &&that) {
    this->renderbuf = that.renderbuf;
    this->texturename = std::move(that.texturename);
    this->attachtarget = that.attachtarget;
    this->multisample = that.multisample;
    this->multisample_texture = that.multisample_texture;
    this->clearonuse = that.clearonuse;
    this->outputnumber = that.outputnumber;
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
    this->outputnumber = that.outputnumber;
    that.renderbuf = 0;
    this->texture = std::move(that.texture);
    return *this;
}

bool RenderAttachment::SystemStart(const std::list<Property> &settings) {
    int width, height, samples;
    for(auto prop : settings) {
        if(prop.GetName() == "screen-width") {
            width = prop.Get<int>();
        }
        else if(prop.GetName() == "screen-height") {
            height = prop.Get<int>();
        }
        else if(prop.GetName() == "multisample") {
            this->multisample = prop.Get<bool>();
        }
        else if(prop.GetName() == "samples") {
            samples = prop.Get<int>();
        }
    }
    Generate(width, height, samples);
    return true;
}
bool RenderAttachment::SystemReset(const std::list<Property> &settings) {
    int width, height, samples;
    for(auto prop : settings) {
        if(prop.GetName() == "screen-width") {
            width = prop.Get<int>();
        }
        else if(prop.GetName() == "screen-height") {
            height = prop.Get<int>();
        }
        else if(prop.GetName() == "multisample") {
            this->multisample = prop.Get<bool>();
        }
        else if(prop.GetName() == "samples") {
            samples = prop.Get<int>();
        }
    }
    Destroy();
    Generate(width, height, samples);
    return true;
}
bool RenderAttachment::Serialize(rapidjson::Document& document) {
    return false;
}

static std::string MakeString(const rapidjson::Value& v) {
    return std::string(v.GetString(), v.GetStringLength());
}

bool RenderAttachment::Parse(const std::string &object_name, rapidjson::Value& node) {
    for(auto attnode = node.MemberBegin(); attnode != node.MemberEnd(); attnode++) {
        std::string attribname = MakeString(attnode->name);
        if(attribname == "texture") {
            if(attnode->value.IsString()) {
                this->texturename = MakeString(attnode->value);
            }
            else {
                // TODO use logger
                std::cerr << "[ERROR] Attachment: Invalid texture name";
                return false;
            }
        }
        else if(attribname == "target") {
            if(attnode->value.IsString()) {
                std::string format = MakeString(attnode->value);
                if(format == "color" || format == "RGBA") {
                    this->attachtarget = GL_COLOR_ATTACHMENT0;
                }
                else if(format == "depth") {
                    this->attachtarget = GL_DEPTH_ATTACHMENT;
                }
                else if(format == "stencil") {
                    this->attachtarget = GL_STENCIL_ATTACHMENT;
                }
                else if(format == "depth-stencil") {
                    this->attachtarget = GL_DEPTH_STENCIL_ATTACHMENT;
                }
            }
            else {
                // TODO use logger
                std::cerr << "[ERROR] Attachment: Invalid target format";
                return false;
            }
        }
        else if(attribname == "number") {
            if(attnode->value.IsUint()) {
                this->outputnumber = attnode->value.GetUint();
            }
            else {
                // TODO use logger
                std::cerr << "[ERROR] Attachment: Invalid target";
                return false;
            }
        }
        else if(attribname == "clear") {
            if(attnode->value.IsArray()) {
                int index = 0;
                for(auto clearelem = attnode->value.MemberBegin(); clearelem != attnode->value.MemberEnd(); attnode++) {
                    if(index > 3) {
                        std::cerr << "[WARNING] Attachment: Excess clearing value";
                        break;
                    }
                    if(clearelem->value.IsNumber()) {
                        this->clearvalues[index] = clearelem->value.GetDouble();
                    }
                    else {
                        std::cerr << "[ERROR] Attachment: Invalid clearing value";
                        return false;
                    }
                    index++;
                }
                this->clearonuse = true;
            }
            else if(attnode->value.IsNumber()) {
                float clearval = attnode->value.GetDouble();
                for(int index = 0; index < 4; index++) {
                    this->clearvalues[index] = clearval;
                }
                this->clearonuse = true;
            }
            else {
                // TODO use logger
                std::cerr << "[ERROR] Attachment: Invalid clear";
                return false;
            }
        }
        else if(attribname == "multisample-texture") {
            if(attnode->value.IsBool()) {
                this->multisample_texture = attnode->value.GetBool();
            }
        }
    }
    return true;
}

void RenderAttachment::Generate(int width, int height, int samplecount) {
    if(multisample && !multisample_texture && renderbuf) {
        return;
    }
    if(texture) {
        if(texture->GetID()) return;
    }
    else {
        auto texptr = TrillekGame::GetGraphicSystem().Get<Texture>(texturename);
        if(texptr) {
            texture = std::move(texptr);
        }
        else {
            texture.reset(new Texture());
            TrillekGame::GetGraphicSystem().Add(texturename, texture);
        }
    }
    if(multisample) {
        if(multisample_texture) {
            texture->GenerateMultisample(width, height, samplecount);
        }
        else {
            glGenRenderbuffers(1, &renderbuf);
            texture->Generate(width, height, true);
        }
    }
    else {
        texture->Generate(width, height, true);
    }
}

void RenderAttachment::Destroy() {
    if(renderbuf) {
        glDeleteRenderbuffers(1, &renderbuf);
        renderbuf = 0;
    }
}
void RenderAttachment::BindTexture() {
    if(texture) {
        glBindTexture(GL_TEXTURE0 + outputnumber, texture->GetID());
    }
}
void RenderAttachment::AttachToFBO() {
    if(renderbuf) {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, this->attachtarget, GL_RENDERBUFFER, renderbuf);
    }
    else {
        if(multisample_texture) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, this->attachtarget, GL_TEXTURE_2D_MULTISAMPLE, this->texture->GetID(), 0);
        }
        else {
            glFramebufferTexture2D(GL_FRAMEBUFFER, this->attachtarget, GL_TEXTURE_2D, this->texture->GetID(), 0);
        }
    }
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
