#if defined(_CLIENT_) || defined(_STANDALONE_)

#include "graphics/render-layer.hpp"
#include "graphics/texture.hpp"
#include "trillek-game.hpp"
#include "systems/graphics.hpp"
#include "logging.hpp"

namespace trillek {
namespace graphics {

RenderAttachment::RenderAttachment() {
    renderbuf = 0;
    this->attachtarget = GL_COLOR_ATTACHMENT0;
    this->multisample = false;
    this->clearonuse = false;
    this->multisample_texture = false;
    this->shadowcompare = false;
    this->outputnumber = 0;
    this->clearstencil = 0;
    this->customsize = false;
    this->width = 0;
    this->height = 0;
    for(int i = 0; i < 4; i++) {
        this->clearvalues[i] = 0;
    }
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
    this->shadowcompare = that.shadowcompare;
    this->clearonuse = that.clearonuse;
    this->outputnumber = that.outputnumber;
    this->clearstencil = that.clearstencil;
    this->customsize = that.customsize;
    this->width = that.width;
    this->height = that.height;
    for(int i = 0; i < 4; i++) {
        this->clearvalues[i] = that.clearvalues[i];
    }
    that.renderbuf = 0;
    this->texture = std::move(that.texture);
}

RenderAttachment& RenderAttachment::operator=(RenderAttachment &&that) {
    this->renderbuf = that.renderbuf;
    this->texturename = std::move(that.texturename);
    this->attachtarget = that.attachtarget;
    this->multisample = that.multisample;
    this->multisample_texture = that.multisample_texture;
    this->shadowcompare = that.shadowcompare;
    this->clearonuse = that.clearonuse;
    this->outputnumber = that.outputnumber;
    this->clearstencil = that.clearstencil;
    this->customsize = that.customsize;
    this->width = that.width;
    this->height = that.height;
    for(int i = 0; i < 4; i++) {
        this->clearvalues[i] = that.clearvalues[i];
    }
    that.renderbuf = 0;
    this->texture = std::move(that.texture);
    return *this;
}

bool RenderAttachment::SystemStart(const std::list<Property> &settings) {
    int samples;
    for(auto prop : settings) {
        if(prop.GetName() == "screen-width") {
            if(!customsize) {
                width = prop.Get<int>();
            }
        }
        else if(prop.GetName() == "screen-height") {
            if(!customsize) {
                height = prop.Get<int>();
            }
        }
        else if(prop.GetName() == "multisample") {
            this->multisample = this->multisample || prop.Get<bool>();
        }
        else if(prop.GetName() == "samples") {
            samples = prop.Get<int>();
        }
    }
    Generate(width, height, samples);
    return true;
}

bool RenderAttachment::SystemReset(const std::list<Property> &settings) {
    int samples;
    for(auto prop : settings) {
        if(prop.GetName() == "screen-width") {
            if(!customsize) {
                width = prop.Get<int>();
            }
        }
        else if(prop.GetName() == "screen-height") {
            if(!customsize) {
                height = prop.Get<int>();
            }
        }
        else if(prop.GetName() == "multisample") {
            this->multisample = this->multisample || prop.Get<bool>();
        }
        else if(prop.GetName() == "samples") {
            samples = prop.Get<int>();
        }
    }
    Destroy();
    Generate(width, height, samples);
    return true;
}

void RenderAttachment::Clear() {
    if(!this->clearonuse) {
        return; // return if we should not clear
    }
    switch(this->attachtarget) {
    case GL_COLOR_ATTACHMENT0:
        if(outputnumber > 0) {
            glDrawBuffer(GetAttach());
        }
        glClearColor(clearvalues[0], clearvalues[1], clearvalues[2], clearvalues[3]);
        if(outputnumber > 0) {
            glClear(GL_COLOR_BUFFER_BIT);
        }
        break;
    case GL_DEPTH_ATTACHMENT:
        glClearDepth(clearvalues[0]);
        break;
    case GL_STENCIL_ATTACHMENT:
        glClearStencil(clearstencil);
        break;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        glClearDepth(clearvalues[0]);
        glClearStencil(clearstencil);
        break;
    }
}

bool RenderAttachment::Serialize(rapidjson::Document& document) {
    return false;
}

/*
 * Example json section
 *
  "attachment":{
    "coloroutput" : {
      "texture" : "newcolortexture",
      "target" : "color", // allowed are color, depth, stencil, depth-stencil
      "number" : 0, // only for color targets
      "clear" : [0, 0, 0, 0] // clear colors
    }
    "coloroutput2" : {
      "texture" : "newcolortexture2",
      "target" : "color",
      "number" : 0,
      "clear" : 0 // clear all colors
    }
    "depthstencil" : {
      "texture" : "newdepthtexture",
      "target" : "depth-stencil",
      "clear" : [0, 0] // clear depth
    }
  }
*/

bool RenderAttachment::Parse(const std::string &object_name, const rapidjson::Value& node) {
    if(!node.IsObject()) {
        LOGMSGC(WARNING) << "Invalid attachment entry";
        return false;
    }
    for(auto attnode = node.MemberBegin(); attnode != node.MemberEnd(); attnode++) {
        std::string attribname = util::MakeString(attnode->name);
        if(attribname == "texture") {
            if(attnode->value.IsString()) {
                this->texturename = util::MakeString(attnode->value);
            }
            else {
                LOGMSGC(ERROR) << "Invalid texture name";
                return false;
            }
        }
        else if(attribname == "target") {
            if(attnode->value.IsString()) {
                std::string format = util::MakeString(attnode->value);
                if(format == "color" || format == "RGBA") {
                    this->attachtarget = GL_COLOR_ATTACHMENT0;
                }
                else if(format == "depth") {
                    this->attachtarget = GL_DEPTH_ATTACHMENT;
                }
                else if(format == "depth-shadow") {
                    this->attachtarget = GL_DEPTH_ATTACHMENT;
                    this->shadowcompare = true;
                }
                else if(format == "stencil") {
                    this->attachtarget = GL_STENCIL_ATTACHMENT;
                }
                else if(format == "depth-stencil") {
                    this->attachtarget = GL_DEPTH_STENCIL_ATTACHMENT;
                }
            }
            else {
                LOGMSGC(ERROR) << "Invalid target format";
                return false;
            }
        }
        else if(attribname == "size") {
            if(attnode->value.IsArray()) {
                if(attnode->value.Size() >= 2) {
                    this->customsize = true;
                    if(attnode->value[0u].IsUint()) {
                        this->width = attnode->value[0u].GetUint();
                    }
                    if(attnode->value[1].IsUint()) {
                        this->height = attnode->value[1].GetUint();
                    }
                }
            }
            else {
                LOGMSGC(ERROR) << "Invalid size";
                return false;
            }
        }
        else if(attribname == "number") {
            if(attnode->value.IsUint()) {
                this->outputnumber = attnode->value.GetUint();
            }
            else {
                LOGMSGC(ERROR) << "Invalid target";
                return false;
            }
        }
        else if(attribname == "clear") {
            if(attnode->value.IsArray()) {
                int index = 0;
                for(auto clearelem = attnode->value.Begin(); clearelem != attnode->value.End(); clearelem++) {
                    if(index > 3) {
                        LOGMSGC(WARNING) << "Excess clearing value";
                        break;
                    }
                    if(clearelem->IsNumber()) {
                        this->clearvalues[index] = clearelem->GetDouble();
                        if(index <= 1 && clearelem->IsInt()) {
                            this->clearstencil = clearelem->GetInt();
                        }
                    }
                    else {
                        LOGMSGC(ERROR) << "Invalid clearing value";
                        return false;
                    }
                    index++;
                }
                this->clearonuse = true;
            }
            else if(attnode->value.IsNumber()) {
                float clearval = attnode->value.GetDouble();
                if(attnode->value.IsInt()) {
                    this->clearstencil = attnode->value.GetInt();
                }
                for(int index = 0; index < 4; index++) {
                    this->clearvalues[index] = clearval;
                }
                this->clearonuse = true;
            }
            else {
                LOGMSGC(ERROR) << "Invalid clear";
                return false;
            }
        }
        else if(attribname == "multisample") {
            if(attnode->value.IsBool()) {
                this->multisample = attnode->value.GetBool();
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
    texture->SetCompare(this->shadowcompare);
    if(multisample) {
        if(multisample_texture) {
            switch(attachtarget) {
            case GL_DEPTH_ATTACHMENT:
                texture->GenerateMultisampleDepth(width, height, samplecount, false);
                break;
            case GL_STENCIL_ATTACHMENT:
                texture->GenerateMultisampleStencil(width, height, samplecount);
                break;
            case GL_DEPTH_STENCIL_ATTACHMENT:
                texture->GenerateMultisampleDepth(width, height, samplecount, true);
                break;
            default:
                texture->GenerateMultisample(width, height, samplecount);\
                break;
            }
        }
        else {
            glGenRenderbuffers(1, &renderbuf);
            switch(attachtarget) {
            case GL_DEPTH_ATTACHMENT:
                texture->GenerateDepth(width, height, false);
                break;
            case GL_STENCIL_ATTACHMENT:
                texture->GenerateStencil(width, height);
                break;
            case GL_DEPTH_STENCIL_ATTACHMENT:
                texture->GenerateDepth(width, height, true);
                break;
            default:
                texture->Generate(width, height, true);\
                break;
            }
        }
    }
    else {
        switch(attachtarget) {
        case GL_DEPTH_ATTACHMENT:
            texture->GenerateDepth(width, height, false);
            break;
        case GL_STENCIL_ATTACHMENT:
            texture->GenerateStencil(width, height);
            break;
        case GL_DEPTH_STENCIL_ATTACHMENT:
            texture->GenerateDepth(width, height, true);
            break;
        default:
            texture->Generate(width, height, true);\
            break;
        }
    }
    GLint tex_target = (multisample_texture ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
    glBindTexture(tex_target, texture->GetID());
    CheckGLError();
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CheckGLError();
    glBindTexture(tex_target, 0);
}

void RenderAttachment::Destroy() {
    if(renderbuf) {
        glDeleteRenderbuffers(1, &renderbuf);
        renderbuf = 0;
    }
}

void RenderAttachment::BindTexture() {
    if(texture) {
        glBindTexture((multisample_texture ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), texture->GetID());
        CheckGLError();
    }
}

void RenderAttachment::AttachToFBO() {
    CheckGLError();
    if(renderbuf) {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, this->GetAttach(), GL_RENDERBUFFER, renderbuf);
        CheckGLError();
    }
    else {
        if(multisample_texture) {
            if(texture) {
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture->GetID());
                CheckGLError();
                glFramebufferTexture2D(GL_FRAMEBUFFER, this->GetAttach(),
                        GL_TEXTURE_2D_MULTISAMPLE, this->texture->GetID(), 0);
                CheckGLError();
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
                CheckGLError();
            }
        }
        else {
            if(texture) {
                glBindTexture(GL_TEXTURE_2D, texture->GetID());
                CheckGLError();
                glFramebufferTexture2D(GL_FRAMEBUFFER, this->GetAttach(),
                        GL_TEXTURE_2D, this->texture->GetID(), 0);
                CheckGLError();
                glBindTexture(GL_TEXTURE_2D, 0);
                CheckGLError();
            }
        }
    }
}

RenderLayer::RenderLayer() {
    initialize_priority = 1; // start after all attachments
    width = 0;
    height = 0;
    fbo_id = 0;
    drawcount = 0;
    clearbits = 0;
    clearany = false;
    customsize = false;
    clearhighbuffers = false;
}

RenderLayer::~RenderLayer() {
    Destroy();
}

static const char * GetFramebufferStatusMessage(GLuint status) {
    switch(status) {
    case GL_FRAMEBUFFER_COMPLETE:
        return "Complete";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        return "Incomplete attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        return "Incomplete multisample buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        return "Missing attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        return "Incomplete layer targets";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        return "Incomplete draw buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        return "Incomplete read buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED:
        return "Not supported (check graphics drivers)";
    }
    return "Unknown error";
}

bool RenderLayer::SystemStart(const std::list<Property> &settings) {
    for(auto prop : settings) {
        if(!customsize) {
            if(prop.GetName() == "screen-width") {
                width = prop.Get<int>();
            }
            else if(prop.GetName() == "screen-height") {
                height = prop.Get<int>();
            }
        }
    }
    this->attachments.clear();
    for(auto attachname : this->attachmentnames) {
        auto attachptr = TrillekGame::GetGraphicSystem().Get<RenderAttachment>(attachname);
        if(attachptr) {
            this->attachments.push_back(attachptr);
        }
        else {
            LOGMSGC(ERROR) << "Attachment not found";
            this->attachments.clear();
            return false;
        }
    }
    Generate();
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id); CheckGLError();
    int i = 0;
    drawcount = 0;
    clearbits = 0;
    clearany = false;
    for(auto& attachptr : this->attachments) {
        attachptr->AttachToFBO();
        if(attachptr->IsColor()) {
            drawcount++;
            // extra steps are required if we have to clear all the color buffers
            if(attachptr->NeedsClear()) {
                clearany = true;
                if(attachptr->GetAttach() != GL_COLOR_ATTACHMENT0) {
                    clearhighbuffers = true;
                }
                else {
                    clearbits |= GL_COLOR_BUFFER_BIT;
                }
            }
        }
        else if(attachptr->NeedsClear()) {
            clearany = true;
            switch(attachptr->GetAttach()) {
            case GL_DEPTH_ATTACHMENT:
                clearbits |= GL_DEPTH_BUFFER_BIT;
                break;
            case GL_STENCIL_ATTACHMENT:
                clearbits |= GL_STENCIL_BUFFER_BIT;
                break;
            case GL_DEPTH_STENCIL_ATTACHMENT:
                clearbits |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
            }
        }
    }
    draworder.reset(new GLenum[drawcount]);
    for(auto& attitr : this->attachments) {
        if(attitr && attitr->IsColor()) {
            draworder.get()[i++] = attitr->GetAttach();
        }
    }
    glDrawBuffers(drawcount, draworder.get()); CheckGLError();

    GLuint status;
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER); CheckGLError();
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        LOGMSGC(ERROR) << "Framebuffer: " << GetFramebufferStatusMessage(status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

bool RenderLayer::SystemReset(const std::list<Property> &settings) {
    return false;
}

bool RenderLayer::Serialize(rapidjson::Document& document) {
    return false;
}

void RenderLayer::Generate() {
    if(fbo_id) return;
    glGenFramebuffers(1, &fbo_id); CheckGLError();
}

void RenderLayer::Destroy() {
    if(fbo_id) {
        glDeleteFramebuffers(1, &fbo_id);
        fbo_id = 0;
    }
}

void RenderLayer::BindToWrite() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id); CheckGLError();
}

void RenderLayer::BindToRender() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id); CheckGLError();
    if(clearany) {
        GLuint attachcount = this->attachments.size();
        unsigned int i, primaryindex = 0;
        for(i = 0; i < attachcount; i++) {
            auto& attitr = this->attachments[i];
            if(attitr) {
                if(attitr->GetAttach() != GL_COLOR_ATTACHMENT0) {
                    attitr->Clear();
                }
                else {
                    primaryindex = i;
                }
            }
        }
        if(clearhighbuffers) {
            // normalize the outputs, since clearing other color buffers changed them
            glDrawBuffers(drawcount, draworder.get()); CheckGLError();
        }
        this->attachments[primaryindex]->Clear(); // the first color buffer just sets values
        glClear(clearbits);
    }
}

void RenderLayer::BindToRead() const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id); CheckGLError();
}

void RenderLayer::BindTextures() const {
    GLuint attachcount = this->attachments.size();
    for(unsigned int i = 0; i < attachcount; i++) {
        auto& attitr = this->attachments[i];
        if(attitr) {
            glActiveTexture(GL_TEXTURE0 + i);
            attitr->BindTexture();
        }
    }
}

void RenderLayer::UnbindFromRead() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); CheckGLError();
}
void RenderLayer::UnbindFromWrite() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); CheckGLError();
}
void RenderLayer::UnbindFromAll() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); CheckGLError();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); CheckGLError();
}

bool RenderLayer::Parse(const std::string &object_name, const rapidjson::Value& node) {
    if(!node.IsObject()) {
        LOGMSGC(WARNING) << "Invalid render layer entry";
        return false;
    }
    for(auto attnode = node.MemberBegin(); attnode != node.MemberEnd(); attnode++) {
        std::string attribname = util::MakeString(attnode->name);
        if(attribname == "attach") {
            if(attnode->value.IsArray()) {
                for(auto attach = attnode->value.Begin(); attach != attnode->value.End(); attach++) {
                    if(attach->IsString()) {
                        this->attachmentnames.push_back(util::MakeString(*attach));
                    }
                    else {
                        LOGMSGC(ERROR) << "Invalid attachment";
                    }
                }
            }
        }
        else if(attribname == "size") {
            if(attnode->value.IsArray()) {
                if(attnode->value.Size() >= 2) {
                    this->customsize = true;
                    if(attnode->value[0u].IsUint()) {
                        this->width = attnode->value[0u].GetUint();
                    }
                    if(attnode->value[1].IsUint()) {
                        this->height = attnode->value[1].GetUint();
                    }
                }
            }
            else {
                LOGMSGC(ERROR) << "Invalid size";
                return false;
            }
        }
    }
    return true;
}

} // namespace graphics
} // namespace trillek

#endif
