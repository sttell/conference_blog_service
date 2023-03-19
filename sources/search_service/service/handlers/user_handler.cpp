#include "user_handler.h"

#include "../../database/user.h"
#include "../../helper.h"

#define GENDER_MALE 1
#define GENDER_FEMALE 2

static bool hasSubstr(const std::string &str, const std::string &substr)
{
    if (str.size() < substr.size())
        return false;
    for (size_t i = 0; i <= str.size() - substr.size(); ++i)
    {
        bool ok{true};
        for (size_t j = 0; ok && (j < substr.size()); ++j)
            ok = (str[i + j] == substr[j]);
        if (ok)
            return true;
    }
    return false;
}

inline bool IsFormHas(const HTMLForm& form, const std::string& field) {
    return form.has(field);
}

inline bool IsFormHas(const HTMLForm& form, std::vector<std::string> fields) {
    bool is_condition_done = true;
    for ( const auto& field : fields ) {
        is_condition_done = is_condition_done && (form.has(field));
    }
    return is_condition_done;
}

inline void ResolveGender(Poco::JSON::Object user_node) {

    if ( user_node.has("gender") ) {
        int gender_value = atoi(user_node.get("gender").toString().c_str());
        std::string new_value;
        switch (gender_value) {
            case GENDER_MALE:   { new_value = "male"; break; }
            case GENDER_FEMALE: { new_value = "female"; break; }
            default:            { new_value = "unknown"; break; }
        }

        user_node.set("gender", new_value);
    }

}

bool UserHandler::check_name(const std::string &name, std::string &reason) {
    if (name.length() < 3)
    {
        reason = "Name must be at leas 3 signs";
        return false;
    }

    if (name.find(' ') != std::string::npos)
    {
        reason = "Name can't contain spaces";
        return false;
    }

    if (name.find('\t') != std::string::npos)
    {
        reason = "Name can't contain spaces";
        return false;
    }

    return true;
}

bool UserHandler::check_email(const std::string &email, std::string &reason) {
    if (email.find('@') == std::string::npos)
    {
        reason = "Email must contain @";
        return false;
    }

    if (email.find(' ') != std::string::npos)
    {
        reason = "EMail can't contain spaces";
        return false;
    }

    if (email.find('\t') != std::string::npos)
    {
        reason = "EMail can't contain spaces";
        return false;
    }

    return true;
}

std::optional<std::string>
UserHandler::ExecuteLoginSearchRequest(HTTPServerRequest& request, [[maybe_unused]] HTTPServerResponse& response) {
    std::optional<std::string> error_description;
    try {
        HTMLForm form(request, request.stream());
        if (IsFormHas(form, "login")) {
            std::string logn = form.get("login");
            auto result = database::User::search(logn);
            if (!result.has_value()) {
                error_description = "User not found.";
                return error_description;
            }

            Poco::JSON::Object json_result = *result.value().toJSON();
            ResolveGender(json_result);

            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");
            std::ostream &ostr = response.send();
            Poco::JSON::Stringifier::stringify(json_result, ostr);
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return error_description;
}

std::optional<std::string>
UserHandler::ExecuteUserSearchRequest(HTTPServerRequest& request, [[maybe_unused]] HTTPServerResponse& response) {
    HTMLForm form(request, request.stream());
    if ( IsFormHas(form, std::vector<std::string>{"first_name", "last_name"}) ) {
        std::cout << "Execute user search by first name and last name mask" << std::endl;
    }
    return std::optional<std::string>{"mock error"};
}

std::optional<std::string>
UserHandler::ExecuteUserAddRequest(HTTPServerRequest& request, [[maybe_unused]] HTTPServerResponse& response) {
    HTMLForm form(request, request.stream());

    std::optional<std::string> error_description;

    if ( IsFormHas(form, std::vector<std::string>{"login", "first_name", "last_name", "email", "gender"}) ) {
        database::User user;
        user.login() = form.get("login");
        user.first_name() = form.get("first_name");
        user.last_name() = form.get("last_name");

        if ( form.has("second_name") ) {
            user.second_name() = form.get("second_name");
        }
        user.email() = form.get("email");

        int gender = 0;
        std::string gender_field = form.get("gender");
        if ( gender_field == "male" ) {
            gender = 1;
        } else if ( gender_field == "female" ) {
            gender = 2;
        } else {
            return "Unexpected gender.";
        }
        user.gender() = gender;

        bool check_result = true;
        std::string message;
        std::string reason;

        if (!check_name(user.get_first_name(), reason)) {
            check_result = false;
            message += reason;
            message += "<br>";
        }

        if (!user.get_second_name().empty() && !check_name(user.get_second_name(), reason)) {
            check_result = false;
            message += reason;
            message += "<br>";
        }

        if (!check_name(user.get_last_name(), reason)) {
            check_result = false;
            message += reason;
            message += "<br>";
        }

        if ( !check_email(user.get_email(), reason) ) {
            check_result = false;
            message += reason;
            message += "<br>";
        }

        if ( check_result ) {
            user.save_to_mysql();
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            response.setChunkedTransferEncoding(true);
            response.setContentType("application/json");
            std::ostream &ostr = response.send();
            ostr << user.get_id();
        } else {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
            std::ostream &ostr = response.send();
            ostr << message;
            response.send();
        }

    }
    return error_description;
}


UserHandler::UserHandler(const std::string &format) : _format(format) { }

void UserHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
    HTMLForm form(request, request.stream());
    std::cout << "Receive URI: " << request.getURI() << std::endl;
    try
    {
        if ( hasSubstr(request.getURI(), "/user") ) {
            if ( request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET ) {
                std::cout << "GET request for URI: " << request.getURI() << std::endl;
                std::optional<std::string> fail_desc = ExecuteLoginSearchRequest(request, response);
                if ( !fail_desc ) {
                    return;
                }

            } else if ( request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST ) {
                std::cout << "POST request for URI: " << request.getURI() << std::endl;
                std::optional<std::string> fail_desc = ExecuteUserAddRequest(request, response);
                if ( !fail_desc ) {
                    return;
                }

            }
        }
        else if ( hasSubstr(request.getURI(), "/search") ) {
            std::cout << "GET request for URI: " << request.getURI() << std::endl;
            std::optional<std::string> fail_desc = ExecuteUserSearchRequest(request, response);
            if ( !fail_desc ) {
                return;
            }
        }
    }
    catch (...) {}

    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("type", "/errors/not_found");
    root->set("title", "Internal exception");
    root->set("status", Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
    root->set("detail", "request ot found");
    root->set("instance", "/user");
    std::ostream &ostr = response.send();
    Poco::JSON::Stringifier::stringify(root, ostr);
}