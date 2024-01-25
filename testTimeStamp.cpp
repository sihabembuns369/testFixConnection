/* from man 3 strftime */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <istream>
std::string timecurrent()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y%m%d-%X", &tstruct);

    return buf;
}

int main(int argc, char *argv[])
{
    char outstr[80];
    time_t t;
    struct tm *tmp;
    const char *fmt = "%Y%m%d-%X";

    t = time(NULL);
    tmp = gmtime(&t);
    strftime(outstr, sizeof(outstr), fmt, tmp);

    printf("%s\n", outstr);
    // exit(EXIT_SUCCESS);
}