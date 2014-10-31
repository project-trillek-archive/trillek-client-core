#ifndef TEXTFILE_H_INCLUDED
#define TEXTFILE_H_INCLUDED

#include <fstream>
#include "systems/resource-system.hpp"

namespace trillek {
namespace resource {

class TextFile : public ResourceBase {
public:
    TextFile() { }
    ~TextFile() { }

    /**
    * \brief Returns a resource with the specified name.
    *
    * The only used initialization property is "filename".
    * \param[in] const std::vector<Property> &properties The creation properties for the resource.
    * \return bool True if initialization finished with no errors.
    */
    virtual bool Initialize(const std::vector<Property> &properties) {
        for (const Property& p : properties) {
            std::string name = p.GetName();
            if (name == "filename") {
                this->filename = p.Get<std::string>();
            }
        }

        std::ifstream f(this->filename, std::ios::in);

        if (f.is_open()) {
            f.seekg(0, std::ios::end);
            this->file_size = f.tellg();
            f.seekg(0);

            this->file_buffer.resize(this->file_size);

            f.read(&this->file_buffer[0], this->file_buffer.size());

            f.close();

            return true;
        }

        return false;
    }

    /**
    * \brief Appends text to the internal buffer.
    *
    * \param[in] const std::string &text The text to ammend to the internal buffer.
    * \return
    */
    void AppendText(const std::string& text) {
        this->file_buffer += text;
    }

    /**
    * \brief Gets a reference to the internal buffer.
    *
    * \return const std::string& The reference to the internal buffer.
    */
    const std::string& GetText() {
        return this->file_buffer;
    }
private:
    std::string file_buffer; // Contents of the text file
    std::string filename; // Relative filename
    std::streamsize file_size; // Size of the file in characters
};

} // End of resource

namespace reflection {

template <> inline const char* GetTypeName<resource::TextFile>() { return "Text"; }
template <> inline unsigned int GetTypeID<resource::TextFile>() { return 1000; }

} // End of reflection
} // End of trillek

#endif
