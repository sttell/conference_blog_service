#include "user_handler.h"

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Base64Decoder.h>

#include "database/user.h"
#include "database/user_role.h"

#include <iostream>
#include <regex>

#define EMAIL_REGEX "^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\\.[a-zA-Z0-9-]+)*$"
#define LOGIN_REGEX "^[a-z0-9_.]{3,16}$"
#define PASSWORD_REGEX "(?=.*\\d)(?=.*[a-z])(?=.*[A-Z])(?!.*\\s).*{8,200}$"

using Poco::Net::HTMLForm;

namespace {

    /**
     * @brief Проверяет наличие необходимых аргументов в HTML форме
     * @param form - HTML форма запроса
     * @param fields - Обязательные поля
     * @return true - если все перечисленные поля находятся в форме, иначе - false
     */
    inline bool IsFormHasRequired(const HTMLForm& form, const std::vector<std::string>& fields) {
        bool is_condition_done = true;
        for ( const auto& field : fields ) {
            is_condition_done = is_condition_done && (form.has(field));
        }
        return is_condition_done;
    }

    /**
     * @brief Проверка имени на валидность.
     * @details Допустимый формат имени - символы A-Z в любом из регистров
     * @param name - имя/фамилия/отчество человека
     * @return true - если имя валидно, иначе - false
     */
    bool IsValidName( const std::string& name ) {

        return std::all_of(name.begin(), name.end(), [](const char& sym){
            return (sym >= 'A' && sym <= 'Z') || (sym >= 'a' && sym <= 'z');
        });

    }

    /**
     * @brief Проверка валидности E-Mail адреса
     * @details валидный формат email - *****@****.***
     * @param email - Email address
     * @return true - если формат валидный, иначе - false
     */
    bool IsValidEmail( const std::string& email ) {

        static const std::regex main_rx(EMAIL_REGEX);

        bool is_contain_spaces = std::any_of(email.begin(), email.end(), [](const char& sym){ return sym == ' '; });
        if ( is_contain_spaces ) return false;

        std::smatch m;
        bool is_found = std::regex_search(email, m, main_rx);

        return is_found;
    }

    /**
     * @brief Проверка валидности указанного пола.
     * @details допустимые варианты - male, female.
     * @param gender - пол
     * @return true - если пол валидный, иначе - false
     */
    bool IsValidGender( const std::string& gender ) {
        if ( gender == "male" || gender == "female" ) return true;
        return false;
    }

    /**
     * @brief Проверка валидности логина
     * @details Валидный логин не содержит пробелов. Разрешены спец символы _ и .
     * @param login - Логин
     * @return true - если логин валидный, иначе - false
     */
    bool IsValidLogin( const std::string& login ) {
        static const std::regex main_rx(LOGIN_REGEX);

        bool is_contain_spaces = std::any_of(login.begin(), login.end(), [](const char& sym){ return sym == ' '; });
        if ( is_contain_spaces ) return false;

        std::smatch m;
        bool is_found = std::regex_search(login, m, main_rx);

        return is_found;
    }

    /**
     * @brief Проверка валидности пароля пользователя.
     * @param password пароль
     * @return true - если пароль валидный, иначе - false
     */
    bool IsValidPassword( const std::string& password ) {
        static const std::regex main_rx(PASSWORD_REGEX);

        bool is_contain_spaces = std::any_of(password.begin(), password.end(), [](const char& sym){ return sym == ' '; });
        if ( is_contain_spaces ) return false;

        std::smatch m;
        bool is_found = std::regex_search(password, m, main_rx);

        return is_found;
    }

    /**
     * @brief Разделение строки на подстроки по разделяющему символу
     * @param str исходная строка
     * @param delimiter разделитель
     * @return вектор подстрок разделенных по delimiter
     */
    std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter=" ") {
        std::string str_copy = str;
        size_t pos;
        std::vector<std::string> tokens;
        while ((pos = str_copy.find(delimiter)) != std::string::npos) {
            tokens.push_back(str_copy.substr(0, pos));
            str_copy.erase(0, pos + delimiter.length());
        }
        tokens.push_back(str_copy);
        return tokens;
    }

} // namespace [ functions ]

namespace handler {

    UserHandler::UserHandler(const std::string &format) : IRequestHandler(format, HandlerType::User) { /* Empty */ }

    /**
     * @brief Общие правила обработки запроса. Перенаправление на конкретную реализацию обработки.
     * @param request - HTTP запрос
     * @param response - HTTP ответ
     */
    void UserHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

        HTMLForm form(request, request.stream());
        try {

            /* Вызов обработчика метода POST для /user/role URI */
            if ( request.getURI().find("/role") != std::string::npos &&
                 request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST ) {

                HandleUserRoleUpdateRequest(request, response);
                return;

            /* Вызов обработчика метода POST для /user URI */
            }else if ( request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST ) {

                HandleUserPostRequest(request, response);
                return;

            /* Вызов обработчика метода GET для /user URI */
            } else if ( request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET ) {

                HandleUserGetRequest(request, response);
                return;

            /* Иные методы запроса на данный URI не поддерживаются */
            } else {
                SetBadRequestResponse(response, "Service unsupported this method for /user URI.");
                return;
            }

        /* При возникновении исключения порождается ответ INTERNAL_ERROR[500] с описанием ошибки */
        } catch (const std::exception& e) {
            std::string error_desc = "Server end of work with exception: ";
            error_desc += e.what();

            std::cerr << error_desc << std::endl;

            SetInternalErrorResponse(response, error_desc);
            return;
        }

    }

    /**
     * @brief Обработка POST запроса на добавление нового пользователя в систему.
     * @param request - HTTP запрос с данными пользователя
     * @param response - HTTP ответ
     */
    void UserHandler::HandleUserPostRequest(Poco::Net::HTTPServerRequest &request,
                                            Poco::Net::HTTPServerResponse &response) {

        HTMLForm form(request, request.stream());

        /* Проверка наличия необзодимых для обработки запроса полей */
        static const std::vector<std::string> required_fields = {
                "first_name",
                "last_name",
                "email",
                "gender",
                "login",
                "password"
        };
        if ( !IsFormHasRequired(form, required_fields) ) {
            SetBadRequestResponse(response, "Wrong request data.");
            return;
        }

        /* Получаем данные из формы */
        std::string first_name = form.get("first_name");
        std::string last_name = form.get("last_name");
        std::string email     = form.get("email");
        std::string gender    = form.get("gender");
        std::string login     = form.get("login");
        std::string password  = form.get("password");
        std::string middle_name;
        if ( form.has("middle_name") ) {
            middle_name = form.get("middle_name");
        }

        /* Проверка валидности данных переданных с клиента */
        if ( !IsValidName(first_name) ) {
            SetBadRequestResponse(response, "Wrong first name format.");
            return;
        }
        if ( !IsValidName(last_name)  ) {
            SetBadRequestResponse(response, "Wrong last name format.");
            return;
        }
        if ( !IsValidName(middle_name) ) {
            SetBadRequestResponse(response, "Wrong middle name format.");
            return;
        }
        if ( !IsValidEmail(email) ) {
            SetBadRequestResponse(response, "Wrong email format.");
            return;
        }
        if ( !IsValidGender(gender) ) {
            SetBadRequestResponse(response, R"(Wrong gender format. Expected "male" or "female")");
            return;
        }
        if ( !IsValidLogin(login) ) {
            SetBadRequestResponse(response, "Wrong login format.");
            return;
        }
        if ( !IsValidPassword(password) ) {
            SetBadRequestResponse(response, "Wrong password format.");
            return;
        }

        /* Создание пользователя */
        database::User user;
        user.FirstName() = first_name;
        user.LastName() = last_name;
        user.MiddleName() = middle_name;
        user.EMail() = email;
        user.Gender() = gender;
        user.Login() = login;
        user.Password() = password;
        user.Role() = database::UserRole(database::UserRole::User);

        /* Проверка наличия пользователя с идентичным логином в системе */
        auto user_with_current_login = database::User::SearchByLogin(login);
        if ( user_with_current_login.has_value() ) {
            SetNotAcceptableResponse(response, "User with current login is exists.");
            return;
        }

        /* Если пользователя с идентичным логином нет, то вставляем в БД */
        user.InsertToDatabase();

        /* Если все успешно отправляем ответ */
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/success");
        root->set("title", "OK");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_OK);
        root->set("instance", "/user");
        root->set("id", std::to_string(user.GetID()));
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);

    }

    /**
     * @brief Обработка GET запроса на добавление нового пользователя в систему.
     * @param request - HTTP запрос с данными пользователя
     * @param response - HTTP ответ
     */
    void UserHandler::HandleUserGetRequest(Poco::Net::HTTPServerRequest &request,
                                           Poco::Net::HTTPServerResponse &response) {

        HTMLForm form(request, request.stream());

        database::User request_sender;

        /* Проверка авторизации. TODO: Вынести в отдельный обработчик запросов. */
        if ( request.hasCredentials() ) {

            std::string scheme;
            std::string base64;
            request.getCredentials(scheme, base64);

            std::stringstream decode_stream;
            decode_stream << base64;
            Poco::Base64Decoder base64_decoder{decode_stream};

            std::string decoded;
            base64_decoder >> decoded;

            auto credentials = SplitString(decoded, ":");
            auto user = database::User::AuthUser(credentials[0], credentials[1]);
            if ( user.has_value() ) {
                request_sender = user.value();
            } else {
                SetBadRequestResponse(response, "Invalid login or password.");
                return;
            }
        } else {
            SetUnauthorizedResponse(response, "User is unauthorized.");
            return;
        }

        /* Проверка доступа к запросу. */
        if ( request_sender.GetRole() <= database::UserRole::User ) {
            SetPermissionDeniedResponse(response, "User does not have privileges for this action");
            return;
        }

        /* Проверка валидности формы */
        if ( !IsFormHasRequired(form, { "id" }) ) {
            SetBadRequestResponse(response, "Wrong request. Form must had id field.");
            return;
        }

        /* Если форма валидна, то обрабатываем */
        long id = atol(form.get("id").c_str());

        /* Если пользователь найден, то возвращаем его данные */
        auto user = database::User::SearchByID(id);
        if ( !user.has_value() ) {
            SetNotFoundResponse(response, "User with requested id not found.");
            return;
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/success");
        root->set("title", "OK");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_OK);
        root->set("instance", "/user");
        auto json_user = user->ToJSON();
        root->set("user", json_user);
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);

    }

    /**
     * @brief Обработка POST запроса на изменение роли пользователя.
     * @param request - HTTP запрос с данными пользователя
     * @param response - HTTP ответ
     */
    void UserHandler::HandleUserRoleUpdateRequest(Poco::Net::HTTPServerRequest &request,
                                                  Poco::Net::HTTPServerResponse &response) {

        database::User request_sender;

        /* Проверка авторизации. TODO: Вынести в отдельный обработчик запросов. */
        if ( request.hasCredentials() ) {

            std::string scheme;
            std::string base64;
            request.getCredentials(scheme, base64);

            std::stringstream decode_stream;
            decode_stream << base64;
            Poco::Base64Decoder base64_decoder{decode_stream};

            std::string decoded;
            base64_decoder >> decoded;

            std::cout << decoded << std::endl;

            auto credentials = SplitString(decoded, ":");
            auto user = database::User::AuthUser(credentials[0], credentials[1]);
            if ( user.has_value() ) {
                request_sender = user.value();
            } else {
                SetBadRequestResponse(response, "Invalid login or password.");
                return;
            }
        } else {
            SetUnauthorizedResponse(response, "User is unauthorized.");
            return;
        }

        /* Проверка доступа к запросу. */
        if ( request_sender.GetRole() < database::UserRole::Administrator ) {
            SetPermissionDeniedResponse(response, "User does not have privileges for this action");
            return;
        }

        HTMLForm form(request);

        /* Проверка валидности формы */
        if ( !IsFormHasRequired(form, { "login", "role" }) ) {
            SetBadRequestResponse(response, "Wrong request. Form must had id field.");
            return;
        }

        /* Парсинг формы */
        std::string login = form.get("login");
        database::UserRole new_role{form.get("role")};
        if ( !IsValidLogin(login) ) {
            SetBadRequestResponse(response, "User login has invalid format.");
            return;
        }

        /* Изменение привелегий */
        auto changed_user = database::User::ChangeRole(login, new_role);
        if ( !changed_user.has_value() ) {
            SetNotFoundResponse(response, "User with requested login not found.");
            return;
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/success");
        root->set("title", "OK");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_OK);
        root->set("instance", "/user");
        root->set("id", std::to_string(changed_user->GetID()));

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);

    }

    /**
     * @brief Заполнение BadRequest(400) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void UserHandler::SetBadRequestResponse(HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/bad_request");
        root->set("title", "Bad request error");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_BAD_REQUEST);
        root->set("detail", description);
        root->set("instance", "/user");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение Unauthorized(401) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void UserHandler::SetUnauthorizedResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/unauthorized_error");
        root->set("title", "User unauthorized.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_UNAUTHORIZED);
        root->set("detail", description);
        root->set("instance", "/user");

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение Forbidden(403) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void
    UserHandler::SetPermissionDeniedResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/forbidden_error");
        root->set("title", "Permission denied.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_FORBIDDEN);
        root->set("detail", description);
        root->set("instance", "/user");

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение NotFound(404) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void UserHandler::SetNotFoundResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_acceptable_error");
        root->set("title", "Not found error.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_NOT_FOUND);
        root->set("detail", description);
        root->set("instance", "/user");

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение NotAcceptable(406) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void
    UserHandler::SetNotAcceptableResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_ACCEPTABLE);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_acceptable_error");
        root->set("title", "Not acceptable error.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_NOT_ACCEPTABLE);
        root->set("detail", description);
        root->set("instance", "/user");

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }


    /**
     * @brief Заполнение InternalError(500) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void UserHandler::SetInternalErrorResponse(Poco::Net::HTTPServerResponse &response,
                                               const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/internal_error");
        root->set("title", "Internal Server Error");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
        root->set("detail", description);
        root->set("instance", "/user");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }


} // namespace handler