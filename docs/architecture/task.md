# Техническое задание к проекту

---

## Основная цель сервиса

Сервис реализует сайт конференции позволяющий пользователям просмотреть списки докладов конференций.

## Общие технические требования

Проект состоит из трех основных компонент:
* Front-end часть. Представляет собой многостраничный сайт.
* Back-end часть. Группа микросервисов объединенные API шлюзом.
* Сервис обеспечивающий непрерывный доступ к хранилищу данных.

## Данные

Проект должен поддерживать работу со следующим списком данных:
* Пользователь:
  * ID
  * Имя
  * Фамилия
  * Отчество
  * E-Mail
  * Пол
  * Логин
  * Пароль
* Доклад:
  * ID
  * ID создателя
  * Название
  * Краткое содержание
  * Полный текст
  * Дата создания
  * Ссылка на внешний источник (опционально)
* Таблица с докладами допущенных до конференции:
  * ID доклада
  * ID пользователя который допустил доклад до конференции
  * Дата допуска

## Требования к API

Основные требования к API Back-End части проекта следующие:
* Авторизация:
  * Создание нового пользователя
  * Авторизация пользователя
* Поисковая система:
  * Поиск пользователя по логину
  * Поиск пользователя по маске "Имя и Фамилия"
* Управление докладами:
  * Создание докладов
  * Удаление докладов
  * Получение списка всех докладов
* Управление конференцией:
  * Добавление доклада в конференцию
  * Удаление доклада из конференции
  * Получение списка всех докладов конференции