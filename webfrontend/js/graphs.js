
const defaultLegendClickHandler = Chart.defaults.plugins.legend.onClick;
class Graph {
	constructor(jsonRPC, auth, lang, home) {
		this.home = home;
		this.lang = lang;
		this.jsonRPC = jsonRPC;
		this.auth = auth;

		this.configResult = {};
		
		this.picker = null;
		this.pickerConfig = {};

		this.detailedGraph = null;
		this.graphData = [];

		this.displayUnits = 0;
		this.displayUnitsProp = {
			"append": " °C",
			"min": 0,
			"max": 50
		}

		// This is a helper varriable to remamber which lable was clicked and is not active
		// unfortunately, there is no property which we can use instead
		// The length of this array should match the numer of lables/datasets
		// pm1, pm2p5, pm4, pm10, nox, voc, temp, rh
		this.isLableActive = [true, true, true, true, true, true, true, true]; 
	}

	run() {
		var that = this;

		//Initialize Device Infos
		$('#tab_graphs').load('html/graphs.html #content', function () {

			$('#download_pdf_button').click(function () {
				that.downloadPDF();
			});

			$('#download_xlsx_button').click(function () {
				that.downloadXLSX({ chart: that.detailedGraph });
			});

			$('#download_csv_button').click(function () {
				that.downloadCSV({ chart: that.detailedGraph });
			});

			that.setup();

		});
	}
	/**
	 * async setup graph function; awaits extension.get config and XMLHttpRequest + parsing of graphData
	 */
	async setup() {
		var that = this;
		
		await that.getGlobalConfig();	// await extension.get config
		await that.getGraphData();		// XMLHttpRequest + parsing of graphData
		that.initGraphs();				// "initGraphs" by either destroying the old one and constructing a new graph 
										// or starting from clean; settings are reset to default here (specified in initGraphs)
		this.initEasypick();
										
		that.setLanguage(that.lang);	// set Language changed all text inside the TAB to the desired language; also inits the datepicker with callbacks
		
		that.getData();						// no function; intended to update the graph with real time data
		
		
	}

	setLanguage(lang) {
		try{
			var that = this;

			that.lang = lang;
			if (lang == "de") {

				$('.lang-en').hide();
				$('.lang-de').show();

				//Update Graph language settings
				that.detailedGraph.data.datasets[0].label = "Pm 1.0";
				that.detailedGraph.data.datasets[1].label = "Pm 2.5";
				that.detailedGraph.data.datasets[2].label = "Pm 4.0";
				that.detailedGraph.data.datasets[3].label = "Pm 10";
				that.detailedGraph.data.datasets[4].label = "NOX";
				that.detailedGraph.data.datasets[5].label = "VOC";
				that.detailedGraph.data.datasets[6].label = "Temperatur";
				that.detailedGraph.data.datasets[7].label = "Luftfeuchte";
				

				that.detailedGraph.options.scales.x.title.text = "Zeit";
				that.detailedGraph.options.scales.y_particle.title.text = "Feinstaub in μg/m3";
				that.detailedGraph.options.scales.y_index.title.text = "Index";
				that.detailedGraph.options.scales.y_temp.title.text = "Temperatur in" + that.displayUnitsProp["append"];
				that.detailedGraph.options.scales.y_rh.title.text = "Luftfeuchte in %";

				//if the datepicker is already alive we destroy it to force internal label update on new instance
				if (that.picker != null) {
					that.picker.destroy();
				}

				//init the new datepicker instance 
				that.pickerConfig.lang = "de-DE";
				that.pickerConfig.locale.apply = "Anwenden";
				that.pickerConfig.locale.cancel = "Abbrechen";
				that.pickerConfig.RangePlugin.locale.one = "Tag";
				that.pickerConfig.RangePlugin.locale.other = "Tage";
				that.pickerConfig.PresetPlugin.customPreset = {
					'letzte Stunde': [that.getLastHour(), new Date()],					// [label] : [ [startDateTime], [endDateTime] ]
					'letzter Tag': [that.getLastDay(), new Date()],
					'letzte Woche': [that.getLastWeek(), new Date()],
					'letzter Monat': [that.getLastMonth(), new Date()],
					'letztes Jahr': [that.getLastYear(), new Date()],
					'alle Daten': [that.getFirstDate(), that.getLastDate()]
				};
				that.picker = new easepick.create(that.pickerConfig);

			} else {
				$('.lang-en').show();
				$('.lang-de').hide();

				//Update Graph language settings
				that.detailedGraph.data.datasets[0].label = "Pm 1.0";
				that.detailedGraph.data.datasets[1].label = "Pm 2.5";
				that.detailedGraph.data.datasets[2].label = "Pm 4.0";
				that.detailedGraph.data.datasets[3].label = "Pm 10";
				that.detailedGraph.data.datasets[4].label = "NOX";
				that.detailedGraph.data.datasets[5].label = "VOC";
				that.detailedGraph.data.datasets[6].label = "Temperature";
				that.detailedGraph.data.datasets[7].label = "Humidity";
				

				that.detailedGraph.options.scales.x.title.text = "Time";
				that.detailedGraph.options.scales.y_particle.title.text = "Particulate matter in μg/m3";
				that.detailedGraph.options.scales.y_index.title.text = "Index";
				that.detailedGraph.options.scales.y_temp.title.text = "Temperature in" + that.displayUnitsProp["append"];
				that.detailedGraph.options.scales.y_rh.title.text = "Humidity in %";

				if (that.picker != null) {
					that.picker.destroy();
				}

				that.pickerConfig.lang = "en-EN";
				that.pickerConfig.locale.apply = "Apply";
				that.pickerConfig.locale.cancel = "Cancel";
				that.pickerConfig.RangePlugin.locale.one = "Day";
				that.pickerConfig.RangePlugin.locale.other = "Days";
				that.pickerConfig.PresetPlugin.customPreset = {
					'last hour': [that.getLastHour(), new Date()],
					'last day': [that.getLastDay(), new Date()],
					'last week': [that.getLastWeek(), new Date()],
					'last month': [that.getLastMonth(), new Date()],
					'last year': [that.getLastYear(), new Date()],
					'all data': [that.getFirstDate(), that.getLastDate()]
				};
				that.picker = new easepick.create(that.pickerConfig);
			}
			that.detailedGraph.update();
		}catch(e){
			console.log("wait for complete load..");
		}
	}
	refresh() {															//on page reload
		this.getConfig();												//get graph specific config (should be awaited); not used yet
		this.setup();													//reload all data and reninit the graphs
	}
	getConfig() {
		var that = this;

		that.jsonRPC.call('extension.get', { 'select': 'graphsConfig' }).then(function (result) {

		});


	}
	getGlobalConfig(){									//get general extensio config
		var that = this;
		return new Promise(function (resolve, reject) {
			that.jsonRPC.call('extension.get', {'select':'config'}).then(function (result) {
	
				that.displayUnits = result["displayUnits"];
	
				that.updateUnits();

				resolve();								//releases the await statement (promise)
	
			});
		});
	}
	getData() {											//get realtime data
		var that = this;

		that.jsonRPC.call('extension.get', { 'select': 'graphsData' }).then(function (result) {

		});

	}
	initEasypick(){
		var that = this;
		this.pickerConfig = {
			element: "#reportranges",		//html id
			css: [
				"tools/easepick/easepick.css"	//css file to be used for styling
			],
			zIndex: 10,
			format: "DD MMM YYYY",														//date time format (german)
			autoApply: false,															//hide or show the apply button in datepicker 
			locale: {},
			setup(picker) {
				picker.on('select', (event) => {										//setup callback to zoom the graph to new selection
					that.detailedGraph.zoomScale('x', {									//if apply is hit
						min: event.detail.start,
						max: event.detail.end
					}, 'default')
				});
			},
			RangePlugin: {															 	//allow for selection of dateranges (not single days) with RangePlugin
				startDate: this.getFirstDate(),											//on opening the datepicker set the current range to what was read from flash
				endDate: this.getLastDate(),
				locale: {}
			},
			PresetPlugin: {},
			plugins: [																	//include plugins
				"RangePlugin",
				"PresetPlugin"
			]
		};
	}
	//only called by setup to fetch Data from flash; all later realtime updates through getData
	async getGraphData() {

		this.graphData = [];		//flush the graphData array

		let url = '/data';			//set XMLHttp url

		try {
			let res = await fetch(url);				//fetch with included await promise
			console.log(res.status); // 200			//log the fetch status
			console.log(res.statusText); // OK

			let arrayBuffer = await res.arrayBuffer();		//await the conversion of binary to arrayBuffer
			await this.parseArrayBuffer(arrayBuffer);		//await the parsing of the array buffer
		}
		catch (error) {
			console.log(error);						//log errors
		}
	}
	parseArrayBuffer(arrayBuffer) {
		var that = this;
		return new Promise(function (resolve, reject) {

			var byteArray = new Uint8Array(arrayBuffer);				//cast the arrayBuffer to a Uint8Array
			// console.log(byteArray.byteLength);

			if (!byteArray) {											//if there is no data fail the promise
				reject();
			}

			let dataview = new DataView(byteArray.buffer);				//open a new DataView to be filled with the byte data

			/**
			 * DataSet:
			 * 0-3 		timestamp
			 * 4-7 		temperature
			 * 8-11 	humidity
			 * 12-15	pm1p0
			 * 16-19	pm2p5
			 * 20-23	pm4p0
			 * 24-27	pm10
			 * 28-31	nox
			 * 32-35	VOC
			 * 36		|ST|ST|ST|ST|S2|S2|S1|S1|
			 * 		MSB										LSB
			 * 			ST = Status Bit
			 * 			S1 = sensor type 1
			 * 			S2 = sensor type 2
			 * 37-40	checksum
			 * 
			 */


			//TODO: check checksum here!!!!

			//set all graph data for one datapoint to be empty and not displayed
			//var copper_rate = null;
			//var silver_rate = null;
			//var silver_thickness_value = null;
			//var copper_thickness_value = null;


			for (var i = 0; i < byteArray.byteLength; i = i + 41) {			//go through all the bytes in steps of 33 Bytes (1 Dataset at a time)

				let timestamp = dataview.getUint32(i, true);				//read the timestamp of current Dataset
				timestamp = timestamp * 1000;

				// if there is more than one hour between the last dataset and now -> insert empty datset to break the graph
				if (that.graphData.length > 0) {
					if ((timestamp - that.graphData[that.graphData.length - 1].x) > 3800 * 1000) {

						//generate a timestep in the graph with null data to force a break in the linechart
						var emptyTimestamp = (((timestamp - that.graphData[that.graphData.length - 1].x) / 2) + that.graphData[that.graphData.length - 1].x);
						that.graphData.push({
							x: emptyTimestamp, copper: null, silver: null, temperature: null, humidity: null,
							silver_thickness: null, copper_thickness: null
						});
					}
				}

				var temperature_value = dataview.getFloat32(i + 4, true);		//read the temperature value of current Dataset

				if(that.displayUnits == 1){			//if imperial units are selected
					temperature_value = temperature_value *1.8 +32; // calculate in °F
				}
				temperature_value = temperature_value.toFixed(1);

				var humidity_value = dataview.getFloat32(i + 8, true).toFixed(1);			//read the other values of the current Dataset
				var pm1p0_value = dataview.getFloat32(i + 12, true).toFixed(1);
				var pm2p5_value = dataview.getFloat32(i + 16, true).toFixed(1);
				var pm4p0_value = dataview.getFloat32(i + 20, true).toFixed(1);
				var pm10p0_value = dataview.getFloat32(i + 24, true).toFixed(1);
				var nox_value = dataview.getFloat32(i + 28, true).toFixed(1);
				var voc_value = dataview.getFloat32(i + 32, true).toFixed(1);

				var status_value = dataview.getUint8(i+36);
				
				/**
				* 28		|ST|ST|ST|ST|S2|S2|S1|S1|
				* 		MSB										LSB
				* 			ST = Status Bit
				* 			S1 = sensor type 1
				* 			S2 = sensor type 2
				*/

				
				//push a new timestep to the graphdata
				that.graphData.push({ x: timestamp, temperature: temperature_value, humidity: humidity_value,
					pm1p0: pm1p0_value, pm2p5: pm2p5_value, pm4p0: pm4p0_value, pm10p0: pm10p0_value,
					nox: nox_value, voc: voc_value });

			}
			//reached end of parseArrayBuffer
			resolve();		//release promise
		});
	}

	saveConfig() {

	}
	disableSet() {
		$('.form_dashboard').attr('disabled', true);
	}

	enableSet() {
		$('.form_dashboard').attr('disabled', false);
	}

	initGraphs() {
		var that = this;

		if(that.detailedGraph != null){						//because setup is also called on refresh (and through this by changing the units)
			that.detailedGraph.destroy();					//destroy the canvas if currently active
		}

		const data = {										//declare the data configuration for the graph
			datasets: [										//following datasets are in use
				{
					// label: 'Pm 1.0',
					backgroundColor: '#B8B8B8',
					borderColor: '#B8B8B8',
					borderWidth: 2,
					radius: 0,
					data: that.graphData,
					yAxisID: 'y_particle',
					parsing: {
						yAxisKey: 'pm1p0'
					}
				},{
					// label: 'Pm 2.5',
					backgroundColor: '#9C9C9C',
					borderColor: '#9C9C9C',
					borderWidth: 2,
					radius: 0,
					data: that.graphData,
					yAxisID: 'y_particle',
					parsing: {
						yAxisKey: 'pm2p5'
					}
				},{
					// label: 'Pm 4.0',
					backgroundColor: '#7F7F7F',
					borderColor: '#7F7F7F',
					borderWidth: 2,
					radius: 0,
					data: that.graphData,
					yAxisID: 'y_particle',
					parsing: {
						yAxisKey: 'pm4p0'
					}
				},{
					// label: 'Pm 10.0',
					backgroundColor: '#636363',
					borderColor: '#636363',
					borderWidth: 2,
					radius: 0,
					data: that.graphData,
					yAxisID: 'y_particle',
					parsing: {
						yAxisKey: 'pm10p0'
					}
				},{
					// label: 'NOX',
					backgroundColor: 'rgb(184,115,51)',
					borderColor: 'rgb(184,115,51)',
					borderWidth: 2,
					radius: 0,
					data: that.graphData,
					yAxisID: 'y_index',
					parsing: {
						yAxisKey: 'nox'
					}
				},
				{
					// label: 'VOC',
					backgroundColor: 'rgb(184,115,51)',
					borderColor: 'rgb(184,115,51)',
					borderWidth: 2,
					radius: 0,
					data: that.graphData,
					yAxisID: 'y_index',
					parsing: {
						yAxisKey: 'voc'
					}
				},
				{
					// label: 'Temperatur',
					backgroundColor: 'rgb(255,0,0)',
					borderColor: 'rgb(255,0,0)',
					borderWidth: 2,
					radius: 0,
					data: that.graphData,
					yAxisID: 'y_temp',						//this dataste should be plotted allong the x-Axis with y values on secondary y axis ticks (right)
					hidden: false,
					parsing: {
						yAxisKey: 'temperature'
					}
				},
				{
					// label: 'Feuchtigkeit',
					backgroundColor: 'rgb(0,0,255)',
					borderColor: 'rgb(0,0,255)',
					borderWidth: 2,
					radius: 0,
					data: that.graphData,
					yAxisID: 'y_rh',					//this dataste should be plotted allong the x-Axis with y values on tertiary y axis ticks (right)
					hidden: false,
					parsing: {
						yAxisKey: 'humidity'
					}
				}
			]
		};

		const config = {											//declare the display and usage configuration for the graph
			type: 'line',											//type is line graph
			data: data,												//for data configuration use the constant data
			plugins: [],							
			options: {
				// Turn off animations and data parsing for performance
				animation: false,
				parsing: false,
				interaction: {										//## interaction settings for tooltip ##
					mode: 'index',										//display tooltip for whole index (also out of graph values fixed issue ??)
					axis: 'x',											//use x axis for slice
					intersect: false									//?? read docu
				},
				scales: {											//## settings for graph scales (Axen) ##
					y_particle: {										//primary y (PARTICLE)
						type: 'linear',									//scaling linear not logarithmic
						min: 0,											//axis low limit
						//max: 2500,										//axis high limit
						title: {									//## settings for axis title ##
							display: true,								//title is displayed
							font: {
								size: 20								//font size is 20 px
							}
						},
						grid: {										
							display: false,							//display no grid lines
						},
						ticks: {
							//filter function for ticks as callback on zoom
							//if the first tick (label on axis) is not 0 (e.g. 0.08) dont display it 
							//if the last tick is not 2500 dont display it
							//callback: (value, index, values) => (((index == (values.length - 1)) & value != 2500) | ((index == 0) & value != 0)) ? "" : value,
						}
					},
					y_index: {											//primary y (NOX & VOC)
						type: 'linear',									//scaling linear not logarithmic
						min: 0,											//axis low limit
						max: 1000,										//axis high limit
						title: {										//## settings for axis title ##
							display: true,								//title is displayed
							font: {
								size: 20								//font size is 20 px
							}
						},
						grid: {										
							display: false,							//display no grid lines
						},
						ticks: {
							//filter function for ticks as callback on zoom
							//if the first tick (label on axis) is not 0 (e.g. 0.08) dont display it 
							//if the last tick is not 2500 dont display it
							//callback: (value, index, values) => (((index == (values.length - 1)) & value != 2500) | ((index == 0) & value != 0)) ? "" : value,
						}
					},
					y_temp: {												// TEMPERATURE
						type: 'linear',
						position: 'right',									//move axis to the right of the graph
						min: that.displayUnitsProp["min"],					//due to unit conversion there is a different min and max for imperial and metric
						max: that.displayUnitsProp["max"],
						title: {
							display: true,
							font: {
								size: 20
							},
							flip: true,
						},
						grid: {
							display: false,
						},
						ticks: {
							//callback also uses the unit min and max for removing unwanted ticks
							callback: (value, index, values) => (((index == (values.length - 1)) & value != that.displayUnitsProp["max"]) | ((index == 0) & value != that.displayUnitsProp["min"])) ? "" : value,
						},
						//display: false,
					},
					y_rh: {													// HUMIDITY
						type: 'linear',
						position: 'right',
						min: 0,
						max: 100,
						title: {
							display: true,
							font: {
								size: 20
							},
							flip: true
						},
						grid: {
							display: false,
						},
						ticks: {
							callback: (value, index, values) => (((index == (values.length - 1)) & value != 100) | ((index == 0) & value != 0)) ? "" : value,
						},
						//display: false,
					},
					x: {
						// min: start,
						// max: end,
						type: 'time',										//special axis type time (used UTC timestamp as value)
						title: {
							display: true,									//display title
						},
						ticks: {											//## axis label (ticks) settings ##
							autoSkip: true,									//autoskip labels if they would overlap
							autoSkipPadding: 50,							//i think it allows up to 50 ticks ?! maybe read docu
							maxRotation: 0									//dont allow the labels to be rotate to up-down
						},
						grid: {
							display: true,									//no grid lines
						},
						time: {
							displayFormats: {								//datetime display formats depending on zoom level
								day: 'yyyy-MM-dd',
								hour: 'yyyy-MM-dd HH:00',
								minute: 'yyyy-MM-dd HH:mm',
								second: 'yyyy-MM-dd HH:mm:ss'
							}
						}
					},
				},
				plugins: {
					decimation: {											//use data decimation to render less datapoints
						enabled: true,										//enabled
						algorithm: 'min-max',								//use algorithm to keep local min and max
					},
					legend: {
						onClick: (e, legendItem, legend) => {

							const index = legendItem.datasetIndex;
							that.isLableActive[index] = !that.isLableActive[index];	// update helper varriable and store active/inactive state of lable
							//console.log(that.isLableActive);

							// hide particle legend, if no particle lable is active
							if (that.isLableActive[0] == false && that.isLableActive[1] == false && that.isLableActive[2] == false && that.isLableActive[3] == false) {
								that.detailedGraph.options.scales.y_particle.display = legendItem.hidden;	//T axis for display handling
							}else{
								that.detailedGraph.options.scales.y_particle.display = legendItem.show;	//T axis for display handling
							}

							// hide index legend, if no index lable is active
							if (that.isLableActive[4] == false && that.isLableActive[5] == false) {
								that.detailedGraph.options.scales.y_index.display = legendItem.hidden;	//T axis for display handling
							}else{
								that.detailedGraph.options.scales.y_index.display = legendItem.show;	//T axis for display handling
							}

							// hide temp legend, if temp lable isn't active
							if (that.isLableActive[6] == false) {
								that.detailedGraph.options.scales.y_temp.display = legendItem.hidden;	//T axis for display handling
							}else{
								that.detailedGraph.options.scales.y_temp.display = legendItem.show;	//T axis for display handling
							}
					
							// hide rh legend, if rh lable isn't active
							if (that.isLableActive[7] == false) {
								that.detailedGraph.options.scales.y_rh.display = legendItem.hidden;	//Rh axis for display handling
							}else{
								that.detailedGraph.options.scales.y_rh.display = legendItem.show;	//T axis for display handling
							}
							defaultLegendClickHandler(e, legendItem, legend);
						},
					},
					zoom: {
						zoom: {																							//## plugin zoom config zoom ##
							wheel: {																						//enable zooming by mouse wheel
								enabled: true,
							},
							pinch: {
								enabled: true,																				//enable zoom by touch pinch (phone)
							},
							mode: 'xy',																						//allow general zoom in x and y axis
							overScaleMode: 'xy',																			//specify that x and y can only be zoomed while mouse of the axis
							drag: {
								enabled: false,																				//disable drag animation inside graph (could be described as select)
								// mode: 'x'
							},
							onZoomComplete({ chart }) {
								//update the picker to have the latest zoom information
								that.picker.setDateRange(new Date(chart.scales.x.min), new Date(chart.scales.x.max));
							}
						},
						pan: {																							//## plugin zoom config zoom ##										
							enabled: true,																					//enable pan
							mode: 'xy',																						//allow general pan in x and y axis
							overScaleMode: 'xy',																			//specify that x and y can only be paned while mouse of the axis
							onPanComplete({ chart }) {
								//update the picker to have the latest zoom information
								that.picker.setDateRange(new Date(chart.scales.x.min), new Date(chart.scales.x.max));
							},
						},
						limits: {																						//## plugin zoom config pan and zoom limits ##		
							x: { min: 'original', max: 'original', minRange: 60 * 60 * 1000 },								//allow the time axis to be paned and zoomed exactly the flash amount
							y_particle: { min: 0, max: 'original', minRange: 5 },
							y_index: { min: 0, max: 1000, minRange: 5 },
							y_temp: { min: that.displayUnitsProp["min"], max: that.displayUnitsProp["max"], minRange: 5 },		//temp to unit max and min
							y_rh: { min: 0, max: 100, minRange: 5 },															//humidity between 0 and 100 %
						},
					},
				},
				transition: {
					zoom: {
						animation: {
							duration: 100																				//short and resource preserving zoom animation
						}
					}
				},

			}
		};

		that.detailedGraph = new Chart(														//generate chart instance								
			document.getElementById('graphs_detailed_graph'),								//use html id
			config																			//use const config as config file
		);
	}

	downloadPDF() {
		
		let font = "Helvetica";
        let text = {
			"en": {
				"file": "Reporting",
				"headding": "Reporting",
				"date":     "Date:",
				"name":     "Device:"
			},
			"de": {
				"file": "Bericht",
				"headding": "Bericht",
				"date":     "Datum:",
				"name":     "Gerät:"
			}
		}
		
        let doc = new jsPDF();
		var now = new Date();
		var timestamp = now.toISOString().split('T')[0];

        doc.setFontSize(20);									    //Headding
        doc.setTextColor("#004388");
        doc.setFontSize(16);
        doc.text(text[this.lang].headding, 20, 20);

		doc.setFontSize(8);                                        // Date
        doc.setFont(font,"normal")
		doc.setTextColor("#000000");
		doc.text(text[this.lang].date + "\n" + text[this.lang].name, 20, 30);
        doc.text(timestamp + "\n" + $("#home_device_subtitle").text(), 34, 30);		// Name

		doc.setFontSize(20);                                        // Device name
        doc.setFont(font, "bold")
		doc.setTextColor("#004388");
        doc.text("ChemWatch S", 144, 20);
        
        doc.setFontSize(16);                                        // SubDevice name
        doc.setFont(font,"normal")
        doc.text("Air corrosivity monitor", 140, 28);

		const canvasImage  = document.getElementById('graphs_detailed_graph');
		doc.addImage(canvasImage, 'JPEG', 20, 50, 170, 90);			// Graph
		
		// do not need the table of GX values here
        //var img = new Image();                                      // Table
        //img.onload = function() { }
        //img.src = 'assets/table_'+this.lang+'.jpg';
        //doc.addImage(img, 'JPEG', 30, 155, 150, 80);	

        var img = new Image();                                      // Bottum Logo
        img.onload = function() { }
        img.src = 'assets/logo_big.jpg';
        doc.addImage(img, 'JPEG', 20, 270, 170, 20);

        var img = new Image();                                      // Copyright
        img.onload = function() { }
        img.src = 'assets/copyright.jpg';
        doc.addImage(img, 'JPEG', 195, 140, 4, 80);
        
        doc.save(text[this.lang].file+'-'+timestamp+'.pdf');			                    // Export

	}

	/**
	 * This is a helper Function to add Chart Data to the CSV/EXCEL download data.
	 * @param {List} list 
	 * @param {Datasets} data 
	 */
	appendDataToList(list, data){

		for (var i = 0; i < data.data.length; i++) {										//go throSugh the whole dataset

			var line_buffer = [];															//go line by line; set like [column1, column2, ...]

			var time = new Date(data.data[i].x);											//get Datetime string from UTC timestamp
			time = time.toLocaleString();
			time = time.replace(',','').split('.');
			if(time[0].length == 1) time[0] = '0'+time[0];
			if(time[1].length == 1) time[1] = '0'+time[1];
			line_buffer.push(time[0]+'.'+time[1]+'.'+time[2]);								//push to first column in array


			if (data.data[i].pm1p0 != null) {
				line_buffer.push(data.data[i].pm1p0);
			} else {
				continue;
			}

			if (data.data[i].pm2p5 != null) {
				line_buffer.push(data.data[i].pm2p5);
			} else {
				continue;
			}

			if (data.data[i].pm4p0 != null) {
				line_buffer.push(data.data[i].pm4p0);
			} else {
				continue;
			}

			if (data.data[i].pm10p0 != null) {
				line_buffer.push(data.data[i].pm10p0);
			} else {
				continue;
			}

			if (data.data[i].nox != null) {
				line_buffer.push(data.data[i].nox);
			} else {
				continue;
			}

			if (data.data[i].voc != null) {
				line_buffer.push(data.data[i].voc);
			} else {
				continue;
			}

			if (data.data[i].temperature != null) {											//push temperature as X.1f or null
				line_buffer.push(data.data[i].temperature);
			} else {
				continue;
			}

			if (data.data[i].humidity != null) {											//push humidity as X.1f or null
				line_buffer.push(data.data[i].humidity);
			} else {
				continue;
			}

			list.push(line_buffer);														//push line into workbook
		}
	}

	downloadXLSX(args) {
		var filename, link;
		const workbook = XLSX.utils.book_new();												//init new workbook of XLSX plugin

		//define the number of columns and the header names for them in one step
		//var ws_data = [["time", "copper rate in Å/30d", "copper thickness in Å", "silver rate in Å/30d", "silver thickness in Å", "temperature in"+this.displayUnitsProp["append"], "humidity in %"]];
		
		var ws_data = [
			["", "Pm 1.0", "Pm 2.5", "Pm 4", "Pm 10", "NOX", "VOC", "Temperature", "Relative humidity"],
			["Local Date DD.MM.YYYY hh:mm:ss", "µg/m³", "µg/m³", "µg/m³", "µg/m³", "Index", "Index", this.displayUnitsProp["append"], "%"]
		]
		
		this.appendDataToList(ws_data, args.chart.data.datasets[0]);						// append chart data to the dataList
		//console.log(ws_data);																//console log

		var worksheet = XLSX.utils.aoa_to_sheet(ws_data);									//convert the array of arrays to a worksheet (one page in XLSX)
		var logs = XLSX.utils.aoa_to_sheet(this.home.events.eventListString);

		XLSX.utils.book_append_sheet(workbook, worksheet, "Device Data");					//add worksheet to workbook (file) and set description
		XLSX.utils.book_append_sheet(workbook, logs, "Events");

		/* set column width */
		worksheet['!cols'] = [
            { width: 30 },  // first column
            { width: 15 },  // second column
            { width: 15 },
            { width: 15 },
            { width: 15 },
            { width: 15 },
            { width: 15 },
			{ width: 25 },
			{ width: 25 }
        ];
		logs['!cols'] = [
            { width: 30 },  // first column
            { width: 60 },  // second column
            { width: 15 }
        ];

		var now = new Date();
		var timestamp = now.toISOString().split('T')[0];

		filename = args.filename || 'chart-data-'+timestamp+'.xlsx';										//use the input filename (later maybe used with settings) or fixed name

		/* create an XLSX file and try to save to Presidents.xlsx */
		XLSX.writeFile(workbook, filename);													//finish file and try download prompt in browser
	}

	downloadCSV(args){

		var csv_data = [
			["", "Pm 1.0", "Pm 2.5", "Pm 4", "Pm 10", "NOX", "VOC", "Temperature", "Relative humidity"],
			["Local Date DD.MM.YYYY hh:mm:ss", "µg/m³", "µg/m³", "µg/m³", "µg/m³", "Index", "Index", this.displayUnitsProp["append"], "%"]
		];

		this.appendDataToList(csv_data, args.chart.data.datasets[0]); // append chart data to the dataList

		// convert csv array data to csv file format
		var csv_file = csv_data.map(function (row) {
			return row.join(',');
		}).join('\n');
				
		var now = new Date();
		var timestamp = now.toISOString().split('T')[0];
		
        var blob = new Blob([csv_file], { type: 'text/csv' });// Blob erstellen
        var fileName = args.filename || 'chart-data-'+timestamp+'.csv';// Dateinamen generieren

        // Download-Link erstellen
        var downloadLink = $('<a>')
            .attr('href', window.URL.createObjectURL(blob))
            .attr('download', fileName)
            .appendTo('body');
        downloadLink[0].click();// Element zum Klicken auslösen
        downloadLink.remove();// Element entfernen

		/*this.home.events.eventListString



		var filename = args.filename || 'chart-data-'+timestamp+'.xlsx';										//use the input filename (later maybe used with settings) or fixed name


		XLSX.writeFile(workbook, filename);		*/
	}

	getLastHour() {																				//utility to get DateTime of 1 hour ago
		let now = new Date();
		return new Date(now.getFullYear(), now.getMonth(), now.getDate(), now.getHours() - 1)
	}
	getLastDay() {																				//utility to get DateTime of 1 day ago 0:00 midnight
		const now = new Date();
		return new Date(now.getFullYear(), now.getMonth(), now.getDate() - 1);
	}
	getLastWeek() {																				//utility to get DateTime of 1 week ago 0:00 midnight
		const now = new Date();
		return new Date(now.getFullYear(), now.getMonth(), now.getDate() - 7);

	}
	getLastMonth() {																			//utility to get DateTime of 1 month ago 0:00 midnight
		const now = new Date();
		return new Date(now.getFullYear(), now.getMonth() - 1, now.getDate());
	}
	getLastYear() {																				//utility to get DateTime of 1 year ago 0:00 midnight
		const now = new Date();
		return new Date(now.getFullYear() - 1, now.getMonth(), now.getDate());
	}
	getFirstDate() {																			//utility to get DateTime of the first datapoint in dataset from flash
		var that = this;
		let timestamp = 0;
		try{
			timestamp = that.graphData[0].x;
		}catch{
			console.log("no datapoints");
		}

		return new Date(timestamp)
	}
	getLastDate() {																				//utility to get DateTime of the last datapoint in dataset from flash
		var that = this
		let timestamp = that.graphData[that.graphData.length - 1].x

		return new Date(timestamp)
	}
	legendFilterFunction(item, chart) {															//filter out the legends for temperature and humidity
		return item.datasetIndex !== 2 && item.datasetIndex !== 3;
	}
	
	updateUnits() {																		//fill in props depending on unit selctions; will take affect on refresh() / setup()
		var that = this;
		if (that.displayUnits == 0) {	//metric

			that.displayUnitsProp["append"] = " °C";
			that.displayUnitsProp["min"] = 0;
			that.displayUnitsProp["max"] = 50;

		} else if (that.displayUnits == 1) {	//imperial

			that.displayUnitsProp["append"] = " °F";
			that.displayUnitsProp["min"] = 32;
			that.displayUnitsProp["max"] = 122;

		}
	}

}

