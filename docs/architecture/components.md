```puml
@startuml
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Container.puml

AddElementTag("microService", $shape=EightSidedShape(), $bgColor="CornflowerBlue", $fontColor="white", $legendText="microservice")
AddElementTag("storage", $shape=RoundedBoxShape(), $bgColor="lightSkyBlue", $fontColor="white")

Person(admin, "Администратор")
Person(moderator, "Модератор")
Person(user, "Пользователь")

System_Ext(web_site, "Клиентский веб-сайт", "HTML, CSS, JavaScript, React", "Веб-интерфейс")

System_Boundary(conference_site, "Серверная часть системы") {
   Container(users_control_service, "Сервис управления пользователями", "С++", "Сервис поиска пользователей", $tags = "microService")
   Container(paper_service, "Сервис докладов", "C++", "Сервис управления докладами", $tags = "microService")
   Container(conference_service, "Сервис конференции", "C++", "Сервис управления конференциями", $tags = "microService")
   Container(content_db_service, "Сервис доступа к контенту", "C++", "Сервис обеспечивающий согласованный доступ к базе контента", $tags = "microService")
   ContainerDb(user_db_proxy, "Proxy к базам данных пользователей", "ProxySQL", "Прослойка обеспечивающая согласованный доступ к распределенному хранилищу", $tags = "storage")
   ContainerDb(user_db_node_1, "База данных пользователей Node 1", "MySQL", "Хранение данных о пользователях", $tags = "storage")
   ContainerDb(user_db_node_2, "База данных пользователей Node 2", "MySQL", "Хранение данных о пользователях", $tags = "storage")
   ContainerDb(content_db, "База данных с контентом", "MySQL", "Хранение данных о конференциях и докладах", $tags = "storage")
}



Rel(admin, web_site, "Просмотр, добавление и редактирование информации о пользователях, конференциях и докладах, управление ролями")
Rel(moderator, web_site, "Модерация контента")
Rel(user, web_site, "Регистрация, просмотр информации о докладах, поиск пользователей")

Rel(web_site, users_control_service, "[HTTPS]: Аутентификация пользователей", "localhost/user_search")
Rel(web_site, paper_service, "[HTTPS]: Управление докладами пользователя", "localhost/paper")
Rel(web_site, conference_service, "[HTTPS]: Управление конференциями", "localhost/conf")

Rel(users_control_service, user_db_proxy, "INSERT/SELECT/UPDATE/DELETE", "SQL")
Rel(user_db_proxy, user_db_node_1, "INSERT/SELECT/UPDATE/DELETE", "SQL")
Rel(user_db_proxy, user_db_node_2, "INSERT/SELECT/UPDATE/DELETE", "SQL")

Rel(auth_service, auth_db, "INSERT/SELECT/UPDATE", "SQL")

Rel(paper_service, content_db_service, "Добавление/удаление/редактирование записей", "HTTPS")
Rel(paper_service, users_control_service, "Авторизация пользователя", "HTTPS")
Rel(conference_service, content_db_service, "Добавление/удаление/редактирование записей", "HTTPS")
Rel(conference_service, users_control_service, "Авторизация пользователя", "HTTPS")
Rel(content_db_service, content_db, "INSERT/SELECT/UPDATE/DELETE", "SQL")
@enduml
```

## Список компонентов

---

### Сервис авторизации

#### API:
* Создание нового пользователя
  * Входные параметры: Login, First Name, Last Name, Middle Name, E-Mail, Gender, Password
  * Выходные параметры: Status, Inserted ID
* Информация о пользователе по ID
  * Входные параметры: ID
  * Выходные параметры: ID, First Name, Last Name, Middle Name, E-Mail, Gender, User Role
* Поиск по маске имя фамилия:
  * Входные параметры: First Name Mask, Last Name Mask
  * Выходные параметры: Array of [ID, First Name, Last Name, Middle Name, E-Mail, Gender, User Role]
* Авторизация пользователя
  * Входные параметры: Login, Password
  * Выходные данные: Status, ID, User Role
* Изменение роли пользователя
  * Входные параметры: Login, New Role
  * Выходные параметры: Changed ID

---
### Сервис управления докладами
#### API:
* Создание доклада:
  * Входные параметры: title, description, content, external link(Can be none)
  * Выходные параметры: Article ID


* Удаление доклада:
  * Входные параметры: Article ID
  * Выходные параметры: Status

* Получение информации о докладе по ID:
  * Входные параметры: Article ID
  * Выходные параметры: Consumer ID, Title, Description, Content, Create Date

* Получение списка всех докладов:
  * Входные параметры: нет
  * Выходные параметры: Array of articles: [Article ID]

---
### Сервис управления конференциями:

#### API:

* Допуск доклада в конференцию
  * Входные данные: Article ID
  * Выходные данные: Status

* Удаление доклада из конференции
  * Входные данные: Article ID
  * Выходные данные: Status

* Получение информации о допуске доклада
  * Входные данные: Article ID
  * Выходные данные: Accept ID, Article ID, Acceptor ID, Accept Date

* Получение списка докладов конференции
  * Входные данные: Conference ID
  * Выходные данные: Array of articles data: [Accept ID, Article ID, Acceptor ID, Accept Date]