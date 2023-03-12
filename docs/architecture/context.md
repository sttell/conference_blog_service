```puml
@startuml
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Container.puml

Person(admin, "Администратор")
Person(moderator, "Модератор")
Person(user, "Пользователь")

System(conference_site, "Сайт конференции", "Веб-сайт для работы с докладами участвующими в конференции")

Rel(admin, conference_site, "Просмотр, добавление и редактирование информации о пользователях, докладах, допуск докладов к конференции")
Rel(moderator, conference_site, "Просмотр, модерация контента и пользователей, допуск докладов к конференции")
Rel(user, conference_site, "Поросмотр контента, поиск пользователей, просмотр докладов, создание и удаление своих докладов")

System_Ext(external_links, "Web Links", "Ссылки на внешние сервисы и ресурсы")
Rel(conference_site, external_links, "Внешние ссылки")

System(backend_system, "Back-end services", "REST-API шлюз предоставляющий доступ к сервисам сайта")
Rel(conference_site, backend_system, "Запросы о предоставлении данных")
Rel(backend_system, conference_site, "Ответы")


@enduml
```

## Назначение систем
| Система          | Описание                                                                     |
|------------------|------------------------------------------------------------------------------|
| Сайт конференции | Веб-интерфейс, обеспечивающий доступ к информации по конференции и докладам. |
| REST API Backend Service | REST API шлюз обеспечивающий доступ к микросервисам, которые предоставляют доступ к данным |