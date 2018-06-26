/*
 * mpit_interface,
 *
 * a c++ wrapper for the MPI_T interface.
 * Copyright (C) 2015 TU Dresden, ZIH
 *
 * Author: Andreas Gocht
 * Email: andreas.gocht [at] tu-dresden.de
 *
 * @brief This file holds an Interface to the MPI-T Interface of MPI-3.0 upwards.
 *
 */

#include "mpit_interface.h"

#include <iostream>

namespace mpit_interface
{
/**
 * translate an MPI-T scope int to a string
 *
 * @param scope scope value to translate
 *
 * @return scope as a string
 */
std::string get_mpit_scope(int scope)
{
    switch (scope)
    {
    case MPI_T_SCOPE_CONSTANT:
        return "MPI_T_SCOPE_CONSTANT";
    case MPI_T_SCOPE_READONLY:
        return "MPI_T_SCOPE_READONLY";
    case MPI_T_SCOPE_LOCAL:
        return "MPI_T_SCOPE_LOCAL";
    case MPI_T_SCOPE_GROUP:
        return "MPI_T_SCOPE_GROUP";
    case MPI_T_SCOPE_GROUP_EQ:
        return "MPI_T_SCOPE_GROUP_EQ";
    case MPI_T_SCOPE_ALL:
        return "MPI_T_SCOPE_ALL";
    case MPI_T_SCOPE_ALL_EQ:
        return "MPI_T_SCOPE_ALL_EQ";
    default:
        return "unkown";
    }
}

/**
 * translate an MPI-T error int to a string
 *
 * @param error error value to translate
 *
 * @return error as a string
 */
std::string get_mpit_error(int error)
{
    switch (error)
    {
    case MPI_SUCCESS:
        return "MPI_SUCCESS";
    case MPI_T_ERR_MEMORY:
        return "MPI_T_ERR_MEMORY";
    case MPI_T_ERR_NOT_INITIALIZED:
        return "MPI_T_ERR_MEMORY";
    case MPI_T_ERR_CANNOT_INIT:
        return "MPI_T_ERR_NOT_INITIALIZED";
    case MPI_T_ERR_INVALID_INDEX:
        return "MPI_T_ERR_INVALID_INDEX";
    case MPI_T_ERR_INVALID_ITEM:
        return "MPI_T_ERR_INVALID_ITEM";
    case MPI_T_ERR_INVALID_HANDLE:
        return "MPI_T_ERR_INVALID_HANDLE";
    case MPI_T_ERR_OUT_OF_HANDLES:
        return "MPI_T_ERR_OUT_OF_HANDLES";
    case MPI_T_ERR_OUT_OF_SESSIONS:
        return "MPI_T_ERR_OUT_OF_SESSIONS";
    case MPI_T_ERR_INVALID_SESSION:
        return "MPI_T_ERR_INVALID_SESSION";
    case MPI_T_ERR_CVAR_SET_NOT_NOW:
        return "MPI_T_ERR_CVAR_SET_NOT_NOW";
    case MPI_T_ERR_CVAR_SET_NEVER:
        return "MPI_T_ERR_CVAR_SET_NEVER";
    case MPI_T_ERR_PVAR_NO_STARTSTOP:
        return "MPI_T_ERR_PVAR_NO_STARTSTOP";
    case MPI_T_ERR_PVAR_NO_WRITE:
        return "MPI_T_ERR_PVAR_NO_WRITE";
    case MPI_T_ERR_PVAR_NO_ATOMIC:
        return "MPI_T_ERR_PVAR_NO_ATOMIC";
#ifndef MPICH /*doesnt looks like mpich knows this two ... hoverver openmpi 1.8.7 knows */
    case MPI_T_ERR_INVALID:
        return "MPI_T_ERR_INVALID";
    case MPI_T_ERR_INVALID_NAME:
        return "MPI_T_ERR_INVALID_NAME";
#endif
    default:
        return "Unknown code";
    }
}
/**
 * translate an MPI-T bind int to a string
 *
 * @param bind bind value to translate
 *
 * @return bind as a string
 */
std::string get_mpit_bind(int bind)
{
    switch (bind)
    {
    case MPI_T_BIND_NO_OBJECT:
        return "MPI_T_BIND_NO_OBJECT";
    case MPI_T_BIND_MPI_COMM:
        return "MPI_T_BIND_MPI_COMM";
    case MPI_T_BIND_MPI_DATATYPE:
        return "MPI_T_BIND_MPI_DATATYPE";
    case MPI_T_BIND_MPI_ERRHANDLER:
        return "MPI_T_BIND_MPI_ERRHANDLER";
    case MPI_T_BIND_MPI_FILE:
        return "MPI_T_BIND_MPI_FILE";
    case MPI_T_BIND_MPI_GROUP:
        return "MPI_T_BIND_MPI_GROUP";
    case MPI_T_BIND_MPI_OP:
        return "MPI_T_BIND_MPI_OP";
    case MPI_T_BIND_MPI_REQUEST:
        return "MPI_T_BIND_MPI_REQUEST";
    case MPI_T_BIND_MPI_WIN:
        return "MPI_T_BIND_MPI_WIN";
    case MPI_T_BIND_MPI_MESSAGE:
        return "MPI_T_BIND_MPI_MESSAGE";
    case MPI_T_BIND_MPI_INFO:
        return "MPI_T_BIND_MPI_INFO";
    default:
        return "Unknown";
    }
}

/**
 * translate an MPI-T data type to a string
 *
 * @param datatype data type value to translate
 *
 * @return datatype as a string
 */
std::string get_mpit_datatype(MPI_Datatype datatype)
{
    if (datatype == MPI_INT)
    {
        return "MPI_INT";
    }
    else if (datatype == MPI_UNSIGNED)
    {
        return "MPI_UNSIGNED";
    }
    else if (datatype == MPI_UNSIGNED_LONG)
    {
        return "MPI_UNSIGNED_LONG";
    }
    else if (datatype == MPI_UNSIGNED_LONG_LONG)
    {
        return "MPI_UNSIGNED_LONG_LONG";
    }
    else if (datatype == MPI_COUNT)
    {
        return "MPI_COUNT";
    }
    else if (datatype == MPI_CHAR)
    {
        return "MPI_CHAR";
    }
    else if (datatype == MPI_DOUBLE)
    {
        return "MPI_DOUBLE";
    }
    else
    {
        return "unknown data type";
    }
}

/**
 * Read the value from a given CVar, and save it into val.
 *
 * @param cvar_index cvar to read
 * @param[out] val return value
 *
 * Throws an mpit_error if an MPI-T returns an error code.
 */
void mpit_values::get_mpit_bind_no_object_int_values(int cvar_index, int *value)
{
    int err;
    int count;
    void *no_object_handel = NULL;
    MPI_T_cvar_handle handle;

    err = MPI_T_cvar_handle_alloc(cvar_index, no_object_handel, &handle, &count);

    if (err != MPI_SUCCESS)
    {
        throw mpit_error(get_mpit_error(err), err);
    }

    /* The following assumes that the variable is */
    /* represented by a single integer */
    err = MPI_T_cvar_read(handle, value);

    if (err != MPI_SUCCESS)
    {
        MPI_T_cvar_handle_free(&handle);
        throw mpit_error(get_mpit_error(err), err);
    }

    err = MPI_T_cvar_handle_free(&handle);
    if (err != MPI_SUCCESS)
    {
        throw mpit_error(get_mpit_error(err), err);
    }
}

/**
 * set the value from a given CVar to val.
 *
 * @param cvar_index CVar to set
 * @param val value to set the CVar to
 *
 * Throws an mpit_error if an MPI-T returns an error code.
 */
void mpit_values::set_mpit_bind_no_object_int_values(int cvar_index, int *value)
{
    int err, count;
    void *no_object_handel = NULL;
    MPI_T_cvar_handle handle;

    err = MPI_T_cvar_handle_alloc(cvar_index, no_object_handel, &handle, &count);

    if (err != MPI_SUCCESS)
    {
        throw mpit_error(get_mpit_error(err), err);
    }
    /* The following assumes that the variable is */
    /* represented by a single integer */
    err = MPI_T_cvar_write(handle, value);

    if (err != MPI_SUCCESS)
    {
        MPI_T_cvar_handle_free(&handle);
        throw mpit_error(get_mpit_error(err), err);
    }

    err = MPI_T_cvar_handle_free(&handle);
    if (err != MPI_SUCCESS)
    {
        throw mpit_error(get_mpit_error(err), err);
    }
}

/**
 * Set the value for the CVar index to the value value.
 *
 * @param index index of CVar to read
 * @param[in|out] value value to set CVar to. On successful exit returns value the old value of
 * CVar.
 *
 * Throws an mpit_error if an MPI-T returns an error code, or if setting CVar is not supported.
 *
 */
void mpit_values::change_mpi_variable(int cvar_index, int *value)
{
    int err;
    char cvar_name[1000];
    int cvar_name_len = 1000;
    int verbosity = 0;
    MPI_Datatype datatype;
    MPI_T_enum enumtype;
    char desc[1000];
    int desc_len = 1000;
    int bind = 0;
    int scope = 0;
    int old_value = 0;
    int new_value = 0;

    // getting informations about the variable
    err = MPI_T_cvar_get_info(cvar_index,
        cvar_name,
        &cvar_name_len,
        &verbosity,
        &datatype,
        &enumtype,
        desc,
        &desc_len,
        &bind,
        &scope);

    if (err != MPI_SUCCESS)
    {
        throw mpit_error(get_mpit_error(err), err);
        return;
    }
    else
    {
        /* check if we support this kind of CVar.*/
        if ((bind == MPI_T_BIND_NO_OBJECT) && (datatype == MPI_INT) &&
            (enumtype == MPI_T_ENUM_NULL))
        {

            get_mpit_bind_no_object_int_values(cvar_index, &old_value);
            set_mpit_bind_no_object_int_values(cvar_index, value);
            get_mpit_bind_no_object_int_values(cvar_index, &new_value);

            if (new_value != *value)
            {
                throw mpit_error(get_mpit_error(err), err);
            }
            else
            {
                *value = old_value;
            }
        }
        else
        {
            /*if we don't support this CVar obtain some additional informations, and throw an
             * error*/
            char enumname[1000];
            int enumname_len = 1000;
            int enumnum = 0;

            if ((enumtype != MPI_T_ENUM_NULL) && (datatype == MPI_INT))
            {
                err = MPI_T_enum_get_info(enumtype, &enumnum, enumname, &enumname_len);
                if (err != MPI_SUCCESS)
                {
                    throw mpit_error(get_mpit_error(err), err);
                }
            }
            else
            {
                strcpy(enumname, "empty");
            }

            throw mpit_error(std::string("error: Not supported: Bind:") + get_mpit_bind(bind) +
                                 std::string(" Datatype: ") + get_mpit_datatype(datatype) +
                                 std::string(" Enumtype: ") + std::string(enumname) +
                                 std::string(" for parameter ") + std::string(cvar_name),
                -1);
        }
    }
}

/**
 * Gets a CVar Id by name. Needed for MPI 3.0. MPI 3.1 specify the a build in function.
 *
 * This function searches all CVars until it finds the CVar with the given name "cvar_name".
 * If there is no CVar with the name "cvar_name" this function throws an "cvar_not_found" exception.
 *
 * @param[in] cvar_name name of the CVar to search for.
 *
 * @return id of the CVar given by "cvar_name"
 *
 */
int mpit_values::get_cvar_by_name(const std::string &cvar_name)
{
    int num_mpi_vars = 0;
    int cvar_id = -1;
    char cvar_info_name[1000];
    int cvar_name_len;
    char desc[1000];
    int desc_len;
    int err, bind, verbose, scope;
    MPI_T_enum enumtype;
    MPI_Datatype datatype;
    err = MPI_T_cvar_get_num(&num_mpi_vars);

    if (err != MPI_SUCCESS)
    {
        throw mpit_error(get_mpit_error(err), err);
    }
    for (int i = 0; i < num_mpi_vars; i++)
    {
        cvar_name_len = 1000;
        desc_len = 1000;
        err = MPI_T_cvar_get_info(i,
            cvar_info_name,
            &cvar_name_len,
            &verbose,
            &datatype,
            &enumtype,
            desc,
            &desc_len, /*no description */
            &bind,
            &scope);
        if (err != MPI_SUCCESS)
        {
            throw mpit_error(get_mpit_error(err), err);
        }
        if (strcmp(cvar_info_name, cvar_name.c_str()) == 0)
        {
            cvar_id = i;
            break;
        }
    }

    if (cvar_id == -1)
    {
        throw cvar_not_found(cvar_name);
    }

    return cvar_id;
}

/* Changes a value by name.
 *
 * This function looks in a internal map for the Id of the given CVar. If the CVar
 * "cvar_name" is not found in the map it calls get_cvar_by_name() and saves the returned
 * value in the local map.
 *
 * @param[in] cvar_name name of the CVar to change.
 * @param[in|out] value value to set CVar to. On successful exit returns value the old value of
 * CVar.
 *
 * @return The mpit id of the requested CVar.
 *
 * Throws a mpit_error if an MPI-T returns an error code, or if setting CVar is not supported.
 * Throws a cvar_not_found exception if there is no CVar with name "cvar_name" is found.
 *
 */
int mpit_values::change_mpi_variable_by_name(std::string cvar_name, int *value)
{
    int cvar_id = 0;
    std::map<std::string, int>::iterator it;

    it = known_cvars.find(cvar_name);

    if (it != known_cvars.end())
    {
        cvar_id = it->second;
    }
    else
    {
        cvar_id = get_cvar_by_name(cvar_name);
        known_cvars.insert(std::pair<std::string, int>(cvar_name, cvar_id));
    }

    change_mpi_variable(cvar_id, value);

    return cvar_id;
}

/** Constructor
 *
 * Creates an instance of mpi_value. This function does the
 * MPI_T_init_thread().
 *
 * You are allowed to have one instance at a time. It is possible
 * to destroy the instance and create a new one.
 *
 * It is allowed to use these class even before MPI_Init().
 * Pleas see MPI specification for details.
 *
 * Throws an mpit_error if an MPI-T returns an error code
 */
mpit_values::mpit_values()
{
    int err;
    int threadsupport;

    err = MPI_T_init_thread(MPI_THREAD_SINGLE, &threadsupport);
    if (err != MPI_SUCCESS)
    {
        throw mpit_error(get_mpit_error(err), err);
    }
}

/** Destructor
 *
 * Deletes the instance. Does MPI_T_finalize().
 *
 * Throws an mpit_error if an MPI-T returns an error code
 */
mpit_values::~mpit_values()
{
    int err;
    err = MPI_T_finalize();
    if (err != MPI_SUCCESS)
    {
        throw mpit_error(get_mpit_error(err), err);
    }
}
}
