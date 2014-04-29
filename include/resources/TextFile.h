#ifndef TEXTFILE_H_INCLUDED
#define TEXTFILE_H_INCLUDED

#include <fstream>

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
                        this->fname = p.Get<std::string>();
                    }
                }

                std::ifstream f(this->fname, std::ios::in);

                if (f.is_open()) {
                    f.seekg(0, std::ios::end);
                    this->fsize = f.tellg();
                    f.seekg(0);

                    this->filebuf.resize(this->fsize);

                    f.read(&this->filebuf[0], this->filebuf.size());

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
                this->filebuf += text;
            }

            /**
            * \brief Gets a reference to the internal buffer.
            *
            * \return const std::string& The reference to the internal buffer.
            */
            const std::string& GetText() {
                return this->filebuf;
            }
        private:
            std::string filebuf; // Contents of the text file
            std::string fname; // Relative filename
            std::streamsize fsize; // Size of the file in characters
        };
    }

    namespace reflection {
        template <> inline const char* GetTypeName<resource::TextFile>() { return "Text"; }
        template <> inline const unsigned int GetTypeID<resource::TextFile>() { return 1000; }
    }
}

#endif
