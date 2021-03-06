#ifndef PLAN_H
#define PLAN_H
#include <string>
#include <vector>

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>


class Plan
{
    public:
        Plan(std::string direccion, std::string usuario, std::string password);
        virtual ~Plan();
        std::vector<std::string> procesar(std::string usuario, const char * params);
    protected:
    private:
        int userId(std::string usuario);
        std::vector<std::string> proyectos(int id);
        std::vector<std::string> help(void);
        std::vector<std::string> hitos(int idUsuario,int idProyecto);
        std::vector<std::string> hitoOK(int idUsuario,int idTarea);
        std::vector<std::string> hitoADD(int idUsuario,std::vector<std::string> argumentos);
        sql::Driver *driver;
        sql::Connection *con;
        sql::Statement *stmt;
        sql::ResultSet *res;
};

#endif // PLAN_H
