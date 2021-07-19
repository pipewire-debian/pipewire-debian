/* $Id: descr_stats.c 530 2010-10-25 14:43:42Z roca $ */
/*
 *  Copyright (c) 2003 INRIA - All rights reserved
 *  (main author: Vincent Roca - vincent.roca@inrialpes.fr)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 *  USA.
 */
/*
 *	This file calculates various statistics on a set of samples:
 *	mean, median, variance, standard deviation, confidence interval
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#ifdef DEBUG
#define TRACE(m)	printf m
#else
#define TRACE(m)
#endif

#define min(a,b)	((a) < (b) ? (a) : (b))
#define max(a,b)	((a) > (b) ? (a) : (b))


#define STEPVAL		0.01	/* default size of an interval for */
				/* histograms. Edit if needed */
#define MAXNAMELEN	256	/* max name string length */
#define MAXCMDLEN	4096	/* max name string length */
#define MAXVAL		1000000	/* max nb of values. Edit if needed */


/*
 * global variables
 */
double	val[MAXVAL];		/* array of values from input file */
int	valnb;			/* nb of values of array */
int	middle;
int	inter_mode = 1;		/* interactive mode flag (1 or 0) */
/* file names and associated file pointers */
char	origifn[MAXNAMELEN];	/* original input file name string */
char	tmpfn[MAXNAMELEN];	/* first temp file name string */
char	statsfn[MAXNAMELEN];	/* stats file name string */
FILE	*statsfp;		/* file ptr */
char	hdatfn[MAXNAMELEN];	/* *histogram.dat full file name string */
char	shorthdatfn[MAXNAMELEN];/* *histogram.dat w/o dir file name string */
char	hdemfn[MAXNAMELEN];	/* *histogram.dem file name string */


/*
 * prototypes
 */
void	usage		(int argc, char *argv[]);
double	confidence_int	(double confidence, double center);
int	compf		(const void *fp1, const void *fp2);
void	initialize_file_names ();


void
usage (int	argc,
       char	*argv[])
{
	printf("\n\
    descr_stats - version 1.2 - July 23rd, 2003\n\
    Copyright (c) 2003 INRIA - All rights reserved\n\
    Author:	vincent.roca@inrialpes.fr\n\
    Web site:	http://www.inrialpes.fr/planete/people/roca/\n\
	\n\
    descr_stats comes with ABSOLUTELY NO WARRANTY; This is free software,\n\
    and you are welcome to redistribute it under certain conditions;\n\
    See the GNU General Public License as published by the Free\n\
    Software Foundation, version 2 or later, for more details.\n\
	\n\
    usage: descr_stats col if [noninter]\n\
	col	 column to consider in the input file (first column is 1).\n\
	if	 the input file.\n\
	noninter run descr_stats in non-interactive mode (false by default).\n\
		 If set, no user interaction is required, and the statistics\n\
		 and histogram files are produced using the default\n\
		 parameters. Files are written to the same directory as if.\n\
	\n\
    This tool calculates various descriptive statistics on a set of\n\
    samples stored in a text file: mean, median, variance, standard\n\
    deviation, confidence interval around the mean and median. It can\n\
    also produce an histogram of the samples. In that case, be careful\n\
    at the step value used, since this parameter may dramatically\n\
    change the results.\n\
    \n\
    External contributors:\n\
    		Timothy M. Lebo\n\
    		Holger Machens\n\
    \n");
	exit(-1);
}


/*
 * find the confidence interval around the given value
 */
double
confidence_int (double	confidence,
		double	center)
{
	int	i_center;
	int	inf, sup;
	int	need;			/* we need so many values... */
	int	nb;			/* ...but we have this number */
	double	delta_inf;
	double	delta_sup;
	int	found = 0;		/* 1 if i_center has been found */

	for (i_center = 0; i_center < valnb - 1; i_center++) {
		if (val[i_center] < center && val[i_center+1] >= center) {
			found++;
			break;
		}
	}
	if (!found) {
		return -1.0;	/* not found */
	}
	need = (int)floor((double)(valnb * confidence));
	nb = 0;
	inf = sup = i_center;
	while (nb < need) {
		if (inf == 0)
			sup++;
		else if (sup == valnb -1)
			inf--;
		else if ((center - val[inf - 1]) < (val[sup + 1] - center))
			inf--;
		else
			sup++;
		nb++;
	}
	delta_inf = center - val[inf];
	delta_sup = val[sup] - center;
	TRACE(("delta_inf=%f, delta_sup=%f, inf=%d, sup=%d, need=%d\n",
		delta_inf, delta_sup, inf, sup, need));
	return (max(delta_inf, delta_sup));
}


/*
 * This function compares two values.
 * It is needed by the qsort() function of the C library.
 * param fp1	first value
 * param fp2	second value
 * return	-1 if if fp1 < fp2; or 0 if fp1 == fp2; or +1 if fp1 > fp2
 */
int
compf  (const void	*fp1,
	const void	*fp2)
{
	double	a = *((double*)fp1);
	double	b = *((double*)fp2);
	double	diff = a - b;

	if (diff < 0)
		return -1;
	else if (diff > 0)
		return +1;
	else
		return 0;
}


int
main (int	argc,
      char	*argv[])
{
	FILE	*ifp;			/* raw input file ptr */
	FILE	*hdatfp;		/* file ptr */
#if DEBUG
	char	sifn[MAXNAMELEN];	/* sorted input file name string */
#endif
	int	col;			/* colomn number to keep */
	char	cmd[MAXCMDLEN];		/* command string */
	int	i;
	double	sum, mean, median, variance, std_deviation, range;
	char	answer;
	double	upto, step, half_step;
	int	nb_per_int;		/* histograms: sample nb per interval */
	double	int_90_mean;
	double	int_90_median;
	double	int_95_mean;
	double	int_95_median;
	double	int_99_mean;
	double	int_99_median;

	if (argc != 3 && argc != 4) {
		if (argc == 2 &&
		    ( (strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "-help") == 0)) ) {
			usage(argc, argv);
		}
		fprintf(stderr,
		"ERROR, bad number of arguments: 3 or 4 expected, got %d\n", argc);
		usage(argc, argv);
	}
	/* parse arguments */
	col = atoi(argv[1]);
	strncpy(origifn, argv[2], MAXNAMELEN);
	if (argc == 4) {
		if (strcmp(argv[3], "noninter") == 0) {
			inter_mode = 0;
		} else {
			fprintf(stderr, "ERROR, bad argument %s\n", argv[3]);
			usage(argc, argv);
		}
	}

	/* initialize all file names and file pointers */
	initialize_file_names();

	/* keep only the appropriate col and store values in temporary file */
	sprintf(cmd, "cat %s | awk '{ print $%d }' > %s", origifn, col, tmpfn);
	TRACE(("--> keep right column cmd: \"%s\"\n", cmd));
	system(cmd);

	/* store values of the input file into our array */
	if ((ifp = fopen(tmpfn, "r")) < 0) {
		perror("open");
		usage(argc, argv);
	}
	valnb = 0;
	while (fscanf(ifp, "%lf", &val[valnb]) == 1) {
		valnb++;
		if (valnb >= MAXVAL) {
			printf("WARNING: limited to %d values, remaining lines ignored\n", MAXVAL);
			break;
		}
	}
	if (valnb <= 1) {
		printf("ERROR: no value read in %s\n", tmpfn);
		exit(-1);
	}
	fclose(ifp);

	/* now sort the array */
	qsort(val, valnb, sizeof(*val), compf);

#ifdef DEBUG
	/* in DEBUG mode store a copy of the sorted table in temp file sifn */
	sprintf(sifn, "/tmp/descr_stats_tmp_%d", getpid()+1);
	if ((ifp = fopen(sifn, "w")) < 0) {
		perror("open");
		usage(argc, argv);
	}
	fprintf(ifp, "Sorted values (%d entries):\n", valnb);
	for (int i = 0; i < valnb; i++) {
		fprintf(ifp, "  %f\n", val[i]);
	}
	fclose(ifp);
#endif // DEBUG

	/* now calculate various stats */
	sum = 0.0;
	for (i = 0; i < valnb; i++) {
		sum += val[i];
	}

	mean = sum / valnb;

	if (valnb % 2) {
		/*
		 * odd nb of samples, the simple case.
		 * Be careful, valnb is the number of elements,
		 * while middle is an index (from 0 to valnb-1)!
		 */
		middle = (int)floor((double)valnb / 2.0);
		median = val[middle];
	} else {
		/*
		 * even nb of samples
		 * val[middle] | median | val[middle+1]
		 * in that case consider the mean between these
		 * two values (val[middle] and val[middle+1]) as
		 * the median.
		 * Be careful, valnb is the number of elements,
		 * while middle is an index (from 0 to valnb-1)!
		 */
		middle = (int)floor((double)(valnb - 1) / 2.0);
		median = (val[middle] + val[middle+1]) / 2.0;
	}

	variance = 0.0;
	for (i = 0; i < valnb; i++) {
		variance += (val[i] - mean) * (val[i] - mean);
	}
	variance = variance / (valnb - 1);
	std_deviation = sqrt(variance);

	range = val[valnb -1] - val[0];

	int_90_mean = confidence_int(0.90, mean);
	int_95_mean = confidence_int(0.95, mean);
	int_99_mean = confidence_int(0.99, mean);
	int_90_median = confidence_int(0.90, median);
	int_95_median = confidence_int(0.95, median);
	int_99_median = confidence_int(0.99, median);

	/*
	 * Print results
	 */
	fprintf(statsfp,
"	------------------------------------------------------\n\
	nb of samples = %d\n\
	mean = %f\n\
	median = %f\n\
	variance = %f\n\
	standard deviation = %f\n\
	range = %f \t min = %f  max = %f\n",
		valnb, mean, median, variance, std_deviation, range,
		val[0], val[valnb -1]);

	fprintf(statsfp, "\tconfidence interval around mean %f:\n", mean);
	if (int_90_mean != -1.0) fprintf(statsfp, "\t\t90: +/- %lf\n", int_90_mean);
	else			 fprintf(statsfp, "\t\t90: NOT FOUND\n");
	if (int_95_mean != -1.0) fprintf(statsfp, "\t\t95: +/- %lf\n", int_95_mean);
	else			 fprintf(statsfp, "\t\t95: NOT FOUND\n");
	if (int_99_mean != -1.0) fprintf(statsfp, "\t\t99: +/- %lf\n", int_99_mean);
	else			 fprintf(statsfp, "\t\t99: NOT FOUND\n");

	fprintf(statsfp, "\tconfidence interval around median %lf:\n", median);
	if (int_90_median != -1.0) fprintf(statsfp, "\t\t90: +/- %lf\n", int_90_median);
	else			   fprintf(statsfp, "\t\t90: NOT FOUND\n");
	if (int_95_median != -1.0) fprintf(statsfp, "\t\t95: +/- %lf\n", int_95_median);
	else			   fprintf(statsfp, "\t\t95: NOT FOUND\n");
	if (int_99_median != -1.0) fprintf(statsfp, "\t\t99: +/- %lf\n", int_99_median);
	else			   fprintf(statsfp, "\t\t99: NOT FOUND\n");
	fprintf(statsfp, "\t------------------------------------------------------\n");
	if (inter_mode == 0) {
		fclose(statsfp);	/* no longer needed */
	}
	/*
	 * Histogram file preparation
	 */
	step = STEPVAL;		/* default value */
	half_step = step * 0.5;	/* default value */
	if (inter_mode == 1) {
		answer = 'n';
		printf("Continue with histogram (produces a gnuplot .dem format) (y/n)[n] ? ");
		scanf("%c", &answer);
		if (answer != 'y')
			goto cleanup;

		printf("Enter sampling step (real number > 0.0): ");
		scanf("%lf", &step);/* XXX: done this way, the user MUST enter a value */
		if (step <= 0.0) {
			printf("ERROR, step value %f invalid (must be > 0)\n", step);
			goto cleanup;
		}
		half_step = step * 0.5;
	}
	printf("histogram data file is:		%s\n", hdatfn);
	printf("histogram gnuplot file is:	%s\n", hdemfn);

	if ((hdatfp = fopen(hdatfn, "w")) == NULL) {
		perror("ERROR, fopen failed for hdatfn");
		exit(-1);
	}

	for (upto = val[0] + step, i = 0, nb_per_int = 0;
	     i < valnb;
	     i++) {
		if (val[i] < upto) {
			nb_per_int ++;
		} else {
			/* switch to next interval */
			TRACE(("histo: %d samples < %f\n", nb_per_int, upto));
			fprintf(hdatfp, "%f %d\n", upto - half_step, nb_per_int);
			upto = val[i] + step;
			nb_per_int = 1;
		}
	}
	/* don't forget the last one... */
	TRACE(("histo: %d samples < %f\n", nb_per_int, upto));
	fprintf(hdatfp, "%f %d\n", upto - half_step, nb_per_int);
	fclose(hdatfp);

	/* and now prepar the *-histogram.dem file */
	sprintf(cmd, "echo \'\
	set title  \"Histogram\"\n\
	set xlabel \"value\"\n\
	set ylabel \"number of samples per %f interval\"\n\
	set autoscale\n\
	set nolabel\n\
	set grid\n\
	plot [] [0:] \"%s\" with impulses\n\
	pause -1 \"Hit return to continue\" \' > %s",
		step, shorthdatfn, hdemfn);
	TRACE(("--> histo.dem creation cmd: \"%s\"\n", cmd));
	system(cmd);

#if 0
	/* and finally launch gnuplot... */
	/*sprintf(cmd, "xterm -e gnuplot -fn 5x8 %s", demfn);*/
	sprintf(cmd, "gnuplot %s", demfn);
	TRACE(("--> gnuplot cmd: \"%s\"\n", cmd));
	system(cmd);
#endif

cleanup:
#ifdef DEBUG
	/* keep temp output files */
#else
	/* remove everything */
	sprintf(cmd, "rm /tmp/descr_stats_tmp_[0-9]*");
	system(cmd);
#endif
	return 0;
}


/*
 * Determine the various file names, according to the mode (interactive
 * or not).
 * Global variables are modified accordingly.
 */
void
initialize_file_names ()
{
	char	dirn[MAXNAMELEN];	/* directory name string where output*/
					/* non-temp files will be placed */
	char    basen[MAXNAMELEN];	/* basename string */
	char	*dot;			/* position of the . in basen string */
	FILE	*file;
	char	cmd[MAXCMDLEN];		/* command string */

	/* determine the various file names required for output files
	 * and fopen() some of them */
	sprintf(tmpfn, "/tmp/descr_stats_tmp_%d", getpid());
	TRACE(("\ttemp file is %s\n", tmpfn));


	/* first we need to retrieve the directory to put the
	 * stats and plot files into */
	sprintf(cmd, "dirname %s", origifn);
	if ((file = popen(cmd, "r")) != NULL) {
		fgets(dirn, MAXNAMELEN, file);
	} else {
		fprintf(stderr, "ERROR, popen(%s) failed.\n", cmd);
		exit(-1);
	}
	pclose(file);
	dirn[strlen(dirn) - 1] = '\0'; /* remove new line char */
	TRACE(("\tdirname is %s\n", dirn));

	/* get the name of the plot file */
	sprintf(cmd, "basename %s", origifn);
	if ((file = popen(cmd, "r")) != NULL) {
		fgets(basen, MAXNAMELEN, file);
	} else {
		fprintf(stderr, "ERROR, popen(%s) failed.\n", cmd);
		exit(-1);
	}
	pclose(file);
	basen[strlen(basen) - 1] = '\0'; /* remove new line char */
	/* remove the extension now */
        dot = strrchr( basen, '.' );
	if (dot != NULL) {
		*dot = '\0';
	}

	sprintf(hdatfn, "%s/%s_histogram.dat", dirn, basen);
	sprintf(shorthdatfn, "%s_histogram.dat", basen);
	sprintf(hdemfn, "%s/%s_histogram.dem", dirn, basen);
	if (inter_mode == 0) {
		/* create the appropriate stats file names / FILE ptr */
		sprintf(statsfn, "%s/%s_stats.txt", dirn, basen);
		statsfp = fopen(statsfn, "w");
	} else {
		/* here statistics are sent to the standard output */
		statsfp = stdout;
	}
	if (statsfp == NULL) {
		fprintf(stderr, "ERROR, opening statsfp failed.\n");
		exit(-1);
	}
}
