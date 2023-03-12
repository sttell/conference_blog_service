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
   Container(search_service, "Сервис поиска", "С++", "Сервис поиска пользователей", $tags = "microService")
   Container(auth_service, "Сервис авторизации", "C++", "Сервис управления пользователями", $tags = "microService")
   Container(paper_service, "Сервис докладов", "C++", "Сервис управления докладами", $tags = "microService")
   Container(conference_service, "Сервис конференций", "C++", "Сервис управления конференциями", $tags = "microService")
   ContainerDb(user_db, "База данных пользователей", "MySQL", "Хранение данных о пользователях", $tags = "storage")
   ContainerDb(content_db, "База данных с контентом", "MySQL", "Хранение данных о конференциях и докладах", $tags = "storage")
}

Rel(admin, web_site, "Просмотр, добавление и редактирование информации о пользователях, конференциях и докладах")
Rel(moderator, web_site, "Модерация контента и пользователей")
Rel(user, web_site, "Регистрация, просмотр информации о конференциях и докладах")

Rel(web_site, search_service, "Поиск пользователей", "localhost/user_search")
Rel(web_site, auth_service, "Авторизация пользователей", "localhost/user_auth")
Rel(web_site, paper_service, "Управление докладами пользователя", "localhost/paper")
Rel(web_site, conference_service, "Управление конференциями", "localhost/conf")


Rel(search_service, user_db, "SELECT", "SQL")
Rel(auth_service, user_db, "INSERT/SELECT/UPDATE", "SQL")

Rel(paper_service, content_db, "INSERT/SELECT/UPDATE/DELETE", "SQL")
Rel(conference_service, content_db, "INSERT/SELECT/UPDATE/DELETE", "SQL")

@enduml
```

## Список компонентов

---

### Сервис авторизации

#### API:
* Создание нового пользователя
  * Входные параметры: Login, First Name, Last Name, Surname, E-Mail, Gender, Password
  * Выходные параметры: Status
* Авторизация пользователя
  * Входные параметры: Login, Password
  * Выходные данные: Status


---
### Сервис поисковой системы

#### API:
* Поиск пользователя по логину
  * Входные параметры: Login
  * Выходные параметры: Status, Name, Firstname, Second name, E-Mail, Gender

* Поиск пользователя по маске "Имя и Фамилия"
  * Входные данные: Name mask, Firstname mask
  * Выходные данные: Status, Name, Firstname, Second name, E-Mail, Gender


---
### Сервис управления докладами
#### API:
* Создание доклада:
  * Входные параметры: Creator login, title, description, content, conference id, external link(Can be none)
  * Выходные параметры: Article ID


* Удаление доклада:
  * Входные параметры: Article ID
  * Выходные параметры: Status


* Получение списка всех докладов:
  * Входные параметры: нет
  * Выходные параметры: Array of articles data: [Creator login, title, description, conference id, external link, create date]

---
### Сервис управления конференциями:

#### API:
* Создание конференции
  * Входные параметры: Creator login, title, description, external link(Can be none)
  * Выходные параметры: Status, Conference ID


* Добавление доклада в конференцию
  * Входные данные: Article ID, Conference ID
  * Выходные данные: Status


* Удаление доклада из конференции
  * Входные данные: Article ID, Conference ID
  * Выходные данные: Status

* Получение списка докладов конференции
  * Входные данные: Conference ID
  * Выходные данные: Array of articles data: [Creator login, title, description, conference id, external link, create date]