#include "plan.h"
#include <cstring>
#include <cstdio>

Plan::Plan(std::string direccion, std::string usuario, std::string password)
{
    driver = get_driver_instance();
    con = driver->connect(direccion, usuario, password);
    con->setSchema("plan");
}

Plan::~Plan()
{
    //dtor
}

std::vector<std::string> Plan::procesar(std::string usuario, const char * params)
{
    char *pch;
    int id;
    std::vector<std::string> salida;

    pch = strtok ((char *)params," ,.-/\r\n");
    pch = strtok (NULL," ,.-/\r\n");
    if(pch == NULL){return salida;}

    id = userId(usuario);

    if(strcmp(pch,"proyectos") == 0)
    {
        salida = proyectos(id);
    }
    else if(strcmp(pch,"tareas") == 0)
    {
        pch = strtok (NULL," ,.-/\r\n");
        if(pch == NULL){return salida;}
        salida = tareas(id,atoi(pch));
    }
    return salida;
}
int Plan::userId(std::string usuario)
{
    int id;
    stmt = con->createStatement();
    char querysql[256];
    sprintf (querysql, "SELECT id from user where name = '%s'\
            ;", usuario.c_str());
    res = stmt->executeQuery(querysql);
    while (res->next()) {
        id = res->getInt(1);
    }
    return id;
}
std::vector<std::string> Plan::proyectos(int id)
{
    std::vector<std::string> salida;
    char querysql[256];

    sprintf (querysql, "SELECT proyecto.id, proyecto.name, proyecto.desc\
        FROM `projekte` as proyecto,`projekte_assigned`\
        as asignado where proyecto.id = asignado.projekt\
        and asignado.user = %d and proyecto.end = 0;", id);
    res = stmt->executeQuery(querysql);
    while (res->next()) {
        salida.push_back("[COLOR=RED]ID:[/COLOR] [B]"+res->getString(1)+"[/B] [COLOR=RED]Proyecto:[/COLOR] [B]" + res->getString(2) + "[/B]");
        salida.push_back(res->getString(3));
    }
    return salida;
}

std::vector<std::string> Plan::tareas(int idUsuario,int idProyecto)
{
    std::vector<std::string> salida;

        char querysql[256];

    sprintf (querysql, "SELECT tasklist.name, tasklist.desc\
        FROM tasklist, projekte_assigned, user\
        WHERE user.id =%d\
        AND tasklist.project =%d\
        AND tasklist.status =1\
        AND projekte_assigned.projekt = tasklist.project\
        AND projekte_assigned.user = user.id", idUsuario,idProyecto);
    res = stmt->executeQuery(querysql);
    while (res->next()) {
        salida.push_back("[COLOR=RED]Tarea:[/COLOR]" + res->getString(1));
        salida.push_back(res->getString(2));
    }


    return salida;
}
