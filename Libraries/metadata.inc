/** Stores metadata about the program, i.e. icon and name.
 * Define variables program_icon[] and program_name{} and then
 * include this file
 */

forward get_program_icon(icon[32]);
forward get_program_name(name{32});

public get_program_icon(icon[32])
    icon = program_icon;

public get_program_name(name{32})
    name = program_name;
