/*
 * mpit_interface,
 *
 * a c++ wrapper for the MPI_T interface.
 * Copyright (C) 2015 TU Dresden, ZIH
 *
 * Author: Andreas Gocht
 * Email: andreas.gocht [at] tu-dresden.de
 *
 */

#ifndef MPIT_INTERFACE_H_
#define MPIT_INTERFACE_H_

#include <map>
#include <mpi.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace mpit_interface
{

class mpit_error : public std::runtime_error
{
public:
    mpit_error(const std::string &what_arg, int rc)
        : std::runtime_error(what_arg + std::string(" failed with code: ") + std::to_string(rc))
    {
    }
};

class cvar_not_found : public std::runtime_error
{
public:
    cvar_not_found(const std::string &cvar_name)
        : std::runtime_error(std::string("CVar \"") + cvar_name + std::string("\" not found"))
    {
    }
};

class mpit_values
{
public:
    mpit_values();
    ~mpit_values();
    void get_mpit_bind_no_object_int_values(int cvar_index, int *value);
    void set_mpit_bind_no_object_int_values(int cvar_index, int *value);
    void change_mpi_variable(int cvar_index, int *value);
    int change_mpi_variable_by_name(std::string cvar_name, int *value);

private:
    int get_cvar_by_name(const std::string &cvar_name);

    std::map<std::string, int> known_cvars;
};

std::string get_mpit_scope(int scope);
std::string get_mpit_error(int error);
std::string get_mpit_bind(int bind);
std::string get_mpit_datatype(MPI_Datatype datatype);
}

#endif /* MPIT_INTERFACE_H_ */
