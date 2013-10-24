/*
 * main.cpp
 *
 * This file is part of FakkuDownloader
 * Copyright (C) 2013 Francesco Nwokeka <francesco.nwokeka@gmail.com>
 */

#include <QtCore/QCoreApplication>

#include "downloader.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (argc != 2) {
        qWarning("SHIT");
        QCoreApplication::quit();
    }

    Downloader *downloader = new Downloader(argv[1]);

    return app.exec();
}
