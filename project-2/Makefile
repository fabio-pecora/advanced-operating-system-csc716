# Makefile for Multithreaded Reader-Writer Problem

all: semaphore global peterson

semaphore:
	g++ -pthread reader_writer.cpp -o reader_writer

global:
	g++ -pthread reader_writer_global.cpp -o reader_writer_global

peterson:
	g++ -pthread reader_writer_peterson.cpp -o reader_writer_peterson

clean:
	rm -f reader_writer reader_writer_global reader_writer
