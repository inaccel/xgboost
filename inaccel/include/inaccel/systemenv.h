#ifndef SYSTEMENV_H_
#define SYSTEMENV_H_

#ifdef __cplusplus
extern "C" {
#endif
/** @brief Extract the value of a given variable.
 *
	 *  @details This method gets a variable and searches in the defined
 *  %file to extract any values.
 *
 *  @param variable The variable's value to be extracted
 *  @returns The variable's extracted value or null pointer if variable
 *  not defined in the file or file does not exist.
 */
char * inaccel_getenv(const char *key);

/** @brief  Extracts the boolean value of the specified environment
 *  variable or defaultValue if this variable is not defined.
 *
 *  @param variable The variable's value to be extracted.
 *  @param defaultValue The default value.
 *  @returns The boolean value.
 */
unsigned char inaccel_getordefault_bool(const char *variable, unsigned char defaultValue);

/** @brief  Extracts the double value of the specified environment
 *  variable or defaultValue if this variable is not defined.
 *
 *  @param variable The variable's value to be extracted.
 *  @param defaultValue The default value.
 *  @returns The double value.
 */
double inaccel_getordefault_double(const char *variable, double defaultValue);

/** @brief  Extracts the float value of the specified environment
 *  variable or defaultValue if this variable is not defined.
 *
 *  @param variable The variable's value to be extracted.
 *  @param defaultValue The default value.
 *  @returns The float value.
 */
float inaccel_getordefault_float(const char *variable, float defaultValue);

/** @brief  Extracts the int value of the specified environment
 *  variable or defaultValue if this variable is not defined.
 *
 *  @param variable The variable's value to be extracted.
 *  @param defaultValue The default value.
 *  @returns The int value.
 */
int inaccel_getordefault_int(const char *variable, int defaultValue);

/** @brief  Extracts the long value of the specified environment
 *  variable or defaultValue if this variable is not defined.
 *
 *  @param variable The variable's value to be extracted.
 *  @param defaultValue The default value.
 *  @returns The long value.
 */
long inaccel_getordefault_long(const char *variable, long defaultValue);

/** @brief  Extracts the string value of the specified environment
 *  variable or defaultValue if this variable is not defined.
 *
 *  @param variable The variable's value to be extracted.
 *  @param defaultValue The default value.
 *  @returns The string value.
 */
const char * inaccel_getordefault_string(const char *variable,const  char *defaultValue);

#ifdef __cplusplus
}
#endif

#endif // SYSTEMENV_H_
