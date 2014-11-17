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
    std::vector<std::string> argumentos;


    pch = strtok ((char *)params," ,.-/\r\n");
    pch = strtok (NULL," ,.-/\r\n");
    if(pch == NULL){return salida;}

    id = userId(usuario);

    if(strcmp(pch,"proyectos") == 0)
    {
        salida = proyectos(id);
    }
    else if(strcmp(pch,"hitos") == 0)
    {
        pch = strtok (NULL," ,.-/\r\n");
        if(pch == NULL){return salida;}
        salida = hitos(id,atoi(pch));
    }
    else if(strcmp(pch,"help") == 0)
    {
        salida = help();
    }
    else if(strcmp(pch,"hitoOK") == 0)
    {
        pch = strtok (NULL," ,.-/\r\n");
        if(pch == NULL){return salida;}
        salida = hitoOK(id,atoi(pch));
    }
    else if(strcmp(pch,"hitoADD") == 0)
    {

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

std::vector<std::string> Plan::hitos(int idUsuario,int idProyecto)
{
    std::vector<std::string> salida;

        char querysql[256];

    sprintf (querysql, "SELECT tasklist.name, tasklist.desc, tasklist.id\
        FROM tasklist, projekte_assigned, user\
        WHERE user.id =%d\
        AND tasklist.project =%d\
        AND tasklist.status =1\
        AND projekte_assigned.projekt = tasklist.project\
        AND projekte_assigned.user = user.id", idUsuario,idProyecto);
    res = stmt->executeQuery(querysql);
    while (res->next()) {
        salida.push_back("[COLOR=RED]ID:[/COLOR] [B]"+res->getString(3)+"[/B] [COLOR=RED]Hito:[/COLOR] [B]" + res->getString(1) + "[/B]");
        salida.push_back(res->getString(2));
    }


    return salida;
}
std::vector<std::string> Plan::hitoOK(int idUsuario,int idHito)
{
    std::vector<std::string> salida;
    char querysql[256];
    int count;
    sprintf (querysql, "update tasklist t\
    join projekte_assigned pa on (t.project = pa.projekt)\
    set status = 0\
    WHERE pa.user = %d  and t.id = %d and t.status = 1", idUsuario,idHito);
    count = stmt->executeUpdate(querysql);

    sprintf (querysql, "se actualizar %d registros", count);
    salida.push_back(querysql);
    return salida;
}

std::vector<std::string> Plan::hitoADD(int idUsuario, std::vector<std::string> argumentos)
{
    std::vector<std::string> salida;
    return salida;
}

std::vector<std::string> Plan::help(void)
{
    std::vector<std::string> salida;
    salida.push_back("[B]proyectos[/B] lista todos los proyectos que tenemos asignados");
    salida.push_back("[B]hitos <ProyectoID>[/B] lista todos los hitos incompletos del proyecto cuyo id es <ID>");
    salida.push_back("[B]hitoOK <HitoID>[/B] Marca al hito cuyo id es <HitoID> como alcanzado");
    salida.push_back("[B]hitoADD <ProyectoID> hito=\"<Titulo>\" desc=\"<Descripcion>\"[/B] Agrega un hito al proyecto <ProyectoID>, con el titulo <Titulo> y la descripcion <Descripcion>");

    return salida;
}


