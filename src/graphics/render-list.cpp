
#include "graphics/render-list.hpp"
#include <iostream>
#include <map>

namespace trillek {
namespace graphics {

bool RenderList::SystemStart(const std::list<Property> &settings) {
    return true;
}

bool RenderList::SystemReset(const std::list<Property> &settings) {
    return true;
}

bool RenderList::Serialize(rapidjson::Document& document) {
    return false;
}

bool RenderList::Parse(const std::string &object_name, const rapidjson::Value& node) {
    if(!node.IsArray()) {
        // TODO use logger
        std::cerr << "[WARNING] Invalid render list - expected array\n";
        return false;
    }
    std::map<std::string, RenderCmd> commandtype;
    commandtype["clear"       ] = RenderCmd::CLEAR_SCREEN;
    commandtype["module"      ] = RenderCmd::MODULE_CMD;
    commandtype["script"      ] = RenderCmd::SCRIPT;
    commandtype["exec"        ] = RenderCmd::SCRIPT; // alias for script
    commandtype["render"      ] = RenderCmd::RENDER;
    commandtype["set-param"   ] = RenderCmd::SET_PARAM;
    commandtype["read-layer"  ] = RenderCmd::READ_LAYER;
    commandtype["write-layer" ] = RenderCmd::WRITE_LAYER;
    commandtype["draw-layer"  ] = RenderCmd::SET_RENDER_LAYER;
    commandtype["copy-layer"  ] = RenderCmd::COPY_LAYER;
    commandtype["bind-layer"  ] = RenderCmd::BIND_LAYER_TEXTURES;
    commandtype["bind-texture"] = RenderCmd::BIND_TEXTURE;
    commandtype["bind-shader" ] = RenderCmd::BIND_SHADER;
    for(auto rlobj = node.Begin(); rlobj != node.End(); rlobj++) {
        if(rlobj->IsObject()) {
            bool validitem = false;
            RenderCmd rtypeval;
            Container rparam;
            std::list<Property> rprops;
            for(auto rloitem = rlobj->MemberBegin(); rloitem != rlobj->MemberEnd(); rloitem++) {
                std::string rli_name(rloitem->name.GetString(), rloitem->name.GetStringLength());

                auto rlitype = commandtype.find(rli_name);
                if(!validitem && rlitype != commandtype.end()) {
                    rtypeval = rlitype->second;
                    validitem = true;
                    if(rloitem->value.IsString()) {
                        rparam = Container(
                            std::string(rloitem->value.GetString(),
                                        rloitem->value.GetStringLength())
                        );
                    }
                    else if(rloitem->value.IsBool()) {
                        rparam = Container(rloitem->value.GetBool());
                    }
                    else if(rloitem->value.IsNull()) {
                        rparam = Container();
                    }
                    else if(rloitem->value.IsUint()) {
                        rparam = Container(rloitem->value.GetUint());
                    }
                    else if(rloitem->value.IsDouble()) {
                        rparam = Container(rloitem->value.GetDouble());
                    }
                    //rparam = std::string(rloitem->value.GetString(), rloitem->value.GetStringLength());
                }
                else {
                    if(rloitem->value.IsString()) {
                        rprops.push_back(Property(
                            rli_name,
                            std::string(rloitem->value.GetString(),
                                        rloitem->value.GetStringLength())
                        ));
                    }
                    else if(rloitem->value.IsBool()) {
                        rprops.push_back(Property(rli_name, rloitem->value.GetBool()));
                    }
                    else if(rloitem->value.IsNull()) {
                        rprops.push_back(Property(rli_name, 0u));
                    }
                    else if(rloitem->value.IsUint()) {
                        rprops.push_back(Property(rli_name, rloitem->value.GetUint()));
                    }
                    else if(rloitem->value.IsDouble()) {
                        rprops.push_back(Property(rli_name, rloitem->value.GetDouble()));
                    }
                }
            }
            render_commands.push_back(RenderCommandItem(rtypeval, std::move(rparam), std::move(rprops)));
        }
        else {
            // TODO use logger
            std::cerr << "[WARNING] Invalid render list entry\n";
        }
    }
    return true;
}


} // namespace graphics
} // namespace trillek
