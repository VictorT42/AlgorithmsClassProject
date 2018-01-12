from imposm.parser import OSMParser
import csv

coords_dict = dict()
ways_array = []

def coord_func(coords):
	for nodeID, lon, lat in coords:
		coords_dict[nodeID] = [lat,lon]

def way_func(ways):
	for wayID, tags, nodes in ways:
		if 'highway' in tags:
			if tags['highway'] in ['service', 'motorway', 'trunk', 'primary', 'secondary', 'tertiary', 'unclassified', 'residential']:
				temp_array = []
				for node in nodes:
					temp_array += [coord for coord in coords_dict[node]]
				ways_array.append([wayID, tags['highway']] + temp_array)
		
p = OSMParser(coords_callback = coord_func)
p.parse('athens_greece.osm')

p = OSMParser(ways_callback = way_func)
p.parse('athens_greece.osm')

csvfile = open('athens.csv', 'wb')

csv_writer = csv.writer(csvfile, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
for row in ways_array:
	csv_writer.writerow(row)

	