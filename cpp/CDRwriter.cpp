//
// Created by vinilco on 12/11/23.
//

#include <future>
#include "CDRwriter.h"

CDRWriter& CDRWriter::getInstance() {
    static CDRWriter instance;
    return instance;
}

void CDRWriter::setPath(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    pathCDR_ = path;
}

void CDRWriter::writeToCDR(const std::string &report) {
    std::lock_guard<std::mutex> lock(mutex_);

    // TODO: make async write
    std::ofstream file(pathCDR_, std::ios_base::app);
    if (file.is_open()) {
        file << report << "\n";
        file.close();
    } else {
        BOOST_LOG_TRIVIAL(debug) << "Can not open CDR";
    }
}
