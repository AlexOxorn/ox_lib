#ifndef _OXORN_FILE_H
#define _OXORN_FILE_H

#include <cstdio>
#include <unistd.h>
#include <string_view>
#include <filesystem>

#define OX_THROW_ON_FAILURE if(failure) throw errno;

namespace ox {
    class file {
        FILE *cfile = nullptr;

        void close_file() {
            if (cfile)
                fclose(cfile);
        }
    public:
        operator FILE*() const {
            return cfile;
        }

        file(const char* filename, const char *mode) {
            cfile = fopen(filename, mode);
            if (cfile == nullptr) throw errno;
        }
        file(int fd, const char *mode) {
            cfile = fdopen(fd, mode);
            if (cfile == nullptr) throw errno;
        }
        file(FILE *fp) : cfile{fp} {}

        file(const std::filesystem::path filepath, const char *mode) {
            cfile = fopen(filepath.c_str(), mode);
            if (cfile == nullptr) throw errno;
        }

        file(const file& other) = delete;
        file& operator=(const file& other) = delete;

        file(file&& other) {
            cfile = other.cfile;
            other.cfile = nullptr;
        };
        file& operator=(file&& other) {
            close_file();
            cfile = other.cfile;
            other.cfile = nullptr;
            return *this;
        };

        ~file() {
            close_file();
        }

        void clearerr() noexcept {
            ::clearerr(cfile);
        }

        bool eof() const noexcept {
            return feof(cfile);
        }

        bool error() const noexcept {
            return ferror(cfile);
        }

        void flush() {
            int failure = fflush(cfile);
            OX_THROW_ON_FAILURE;
        }

        fpos_t getpos() const {
            fpos_t to_return{};
            int failure = fgetpos(cfile, &to_return);
            OX_THROW_ON_FAILURE;
            return to_return;
        }

        size_t read(void *ptr, size_t size, size_t nmemb) {
            return fread(ptr, size, nmemb, cfile);
        }

        void reopen(const char *new_filename, const char *new_mode) {
            cfile = freopen(new_filename, new_mode, cfile);
            if (cfile == nullptr) throw errno;
        }

        void seek(long offset, int whence = SEEK_SET) {
            int failure = fseek(cfile, offset, whence);
            OX_THROW_ON_FAILURE;
        }

        void setpos(const fpos_t pos) {
            int failure = fsetpos(cfile, &pos);
            OX_THROW_ON_FAILURE;
        }

        long tell() const {
            long result = ftell(cfile);
            if (result < 0)
                throw errno;
            return result;
        }

        size_t write(const void *ptr, size_t size, size_t nmemb) {
            return fwrite(ptr, size, nmemb, cfile);
        }

        int getc() {
            return fgetc(cfile);
        }

        std::string gets(int max) {
            std::string to_return;
            to_return.reserve(max);
            char* success = fgets(to_return.data(), max, cfile);
            if (!success) throw errno;
            return to_return;
        }

        void putc(int character) {
            int result = fputc(character, cfile);
            if (result == EOF) throw errno;
        }

        void puts(const char *str) {
            int result = fputs(str, cfile);
            if (result == EOF) throw errno;
        }
    };
}

#undef OX_THROW_ON_FAILURE

#endif
