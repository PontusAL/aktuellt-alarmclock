#include <curl/curl.h>

#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

const std::string PROGRAMME_URL = "https://www.svtplay.se/aktuellt";
const std::string BASE_URL = "https://www.svtplay.se";

static size_t writeToString(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* output = static_cast<std::string*>(userp);
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string fetchPage(const std::string& url) {
    CURL* curl = curl_easy_init();

    if (curl == nullptr) {
        throw std::runtime_error("Failed to initialise curl");
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        std::string error = curl_easy_strerror(result);
        curl_easy_cleanup(curl);
        throw std::runtime_error("HTTP request failed: " + error);
    }

    long statusCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
    curl_easy_cleanup(curl);

    if (statusCode < 200 || statusCode >= 300) {
        throw std::runtime_error("Unexpected HTTP status code: " + std::to_string(statusCode));
    }

    return response;
}

std::string htmlUnescapeBasic(std::string value) {
    size_t position = 0;

    while ((position = value.find("&amp;", position)) != std::string::npos) {
        value.replace(position, 5, "&");
        position += 1;
    }

    return value;
}

std::string removeQueryString(const std::string& url) {
    size_t queryPosition = url.find('?');

    if (queryPosition == std::string::npos) {
        return url;
    }

    return url.substr(0, queryPosition);
}

std::string makeFullUrl(const std::string& href) {
    std::string cleanedHref = htmlUnescapeBasic(href);

    if (cleanedHref.rfind("https://", 0) == 0 || cleanedHref.rfind("http://", 0) == 0) {
        return removeQueryString(cleanedHref);
    }

    if (!cleanedHref.empty() && cleanedHref[0] == '/') {
        return removeQueryString(BASE_URL + cleanedHref);
    }

    return removeQueryString(BASE_URL + "/" + cleanedHref);
}

std::string findLatestEpisodeUrl(const std::string& html) {
    std::regex anchorRegex("<a\\b(?=[^>]*data-rt=\"top-area-play-button\")[^>]*>");
    std::smatch anchorMatch;

    if (!std::regex_search(html, anchorMatch, anchorRegex)) {
        throw std::runtime_error("Could not find top-area play button anchor");
    }

    std::string anchorTag = anchorMatch.str();

    std::regex hrefRegex("href=\"([^\"]+)\"");
    std::smatch hrefMatch;

    if (!std::regex_search(anchorTag, hrefMatch, hrefRegex)) {
        throw std::runtime_error("Could not find href in top-area play button anchor");
    }

    return makeFullUrl(hrefMatch[1].str());
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    try {
        std::string html = fetchPage(PROGRAMME_URL);
        std::string episodeUrl = findLatestEpisodeUrl(html);

        std::cout << episodeUrl << std::endl;

        curl_global_cleanup();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << std::endl;

        curl_global_cleanup();
        return 1;
    }
}