// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "stdio.h"
#include "app/mainapp.h"

int main(int argc, char** argv)
{
	int app = application(argc, argv);

	system("pause");

	return app;
}