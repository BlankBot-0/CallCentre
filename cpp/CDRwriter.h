//
// Created by vinilco on 12/11/23.
//

#ifndef PROJECTNAME_CDRWRITER_H
#define PROJECTNAME_CDRWRITER_H

#include "RequestQueue.h"
#include <fstream>
#include <iostream>

class CDRWriter {
public:
    static CDRWriter& getInstance();

    // Sets the path for the CDR
    void setPath(const std::string& path);

    // Writes information from a Call instance to the CDR
    void writeToCDR(const std::string& report);

private:
    CDRWriter() = default;
    ~CDRWriter() = default;

    CDRWriter(const CDRWriter&) = delete;
    CDRWriter& operator=(const CDRWriter&) = delete;
    CDRWriter(CDRWriter &&) = delete;
    CDRWriter& operator=(CDRWriter&&) = delete;

    std::string pathCDR_;
    std::mutex mutex_;
};

#endif //PROJECTNAME_CDRWRITER_H