#include "F_OPCheck/FileWriter.h"
#include <iostream>

FileWriter::FileWriter(std::vector<std::unique_ptr<IWriteCheck>> checks)
    : checks_(std::move(checks)) {}

void FileWriter::write(const std::string& data) {
    for (const auto& c : checks_) {
        if (!c->check()) {
            std::cerr << "Check fehlgeschlagen: " << c->name() << '\n';
            return;
        }
    }
    std::cout << "[FileWriter] " << data << '\n';
}
