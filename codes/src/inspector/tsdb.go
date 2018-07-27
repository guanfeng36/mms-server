package main

import (
	//"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"strings"
	"time"
)

var tsdbClient *Client

func InitTsdb() (err error) {
	tsdbClient, err = NewClient(Options{GlobalConfig.TSDB_ADDR, GlobalConfig.TSDB_USER, GlobalConfig.TSDB_PWD, time.Duration(GlobalConfig.TSDB_TIMEOUT) * time.Second})
	return err
}

type Options struct {
	Addr    string
	User    string
	Pwd     string
	Timeout time.Duration
}

type Client struct {
	url        *url.URL
	httpClient *http.Client
	tr         *http.Transport
}

func NewClient(opt Options) (*Client, error) {
	u, err := url.Parse(fmt.Sprintf("http://%s/db/collectd/series?u=%s&p=%s", opt.Addr, opt.User, opt.Pwd))
	if err != nil {
		return nil, err
	}

	tr := &http.Transport{}

	return &Client{
		url: u,
		httpClient: &http.Client{
			Timeout:   opt.Timeout,
			Transport: tr,
		},
		tr: tr,
	}, nil
}

func (c *Client) Close() error {
	c.tr.CloseIdleConnections()
	return nil
}

type Query struct {
	Aggregator string            `json:"aggregator"`
	Metric     string            `json:"metric"`
	Rate       bool              `json:"rate,omitempty"`
	Tags       map[string]string `json:"tags,omitempty"`
}

type QueryParams struct {
	Start             interface{} `json:"start"`
	End               interface{} `json:"end,omitempty"`
	Queries           []Query     `json:"queries,omitempty"`
	NoAnnotations     bool        `json:"no_annotations,omitempty"`
	GlobalAnnotations bool        `json:"global_annotations,omitempty"`
	MsResolution      bool        `json:"ms,omitempty"`
	ShowTSUIDs        bool        `json:"show_tsuids,omitempty"`
	ShowSummary       bool        `json:"show_summary,omitempty"`
	ShowQuery         bool        `json:"show_query,omitempty"`
	Delete            bool        `json:"delete,omitempty"`
}

type Result struct {
	Metric        string             `json:"metric"`
	Tags          map[string]string  `json:"tags"`
	AggregateTags []string           `json:"aggregateTags"`
	Dps           map[string]float64 `json:"dps"`
}

type InnerError struct {
	Code    int    `json:"code"`
	Message string `json:"message"`
	Details string `json:"details"`
}

type PointsItemWithNum struct {
	Time   float64
	Seqnum float64
	Value  float64
}

type MessageWithNum struct {
	Name    string
	Columns []string
	Points  []PointsItemWithNum
}

type PointsItemNoNum struct {
	Time  float64
	Value float64
}

type MessageNoNum struct {
	Name    string
	Columns []string
	Points  []PointsItemNoNum
}

type ErrorResp struct {
	Error InnerError `json:"error"`
}

func (this ErrorResp) String() string {
	return fmt.Sprintf("{code: %d, message: %s, detail: %s}", this.Error.Code, this.Error.Message, this.Error.Details)
}

func (p *PointsItemWithNum) UnmarshalJSON(data []byte) error {
	var v [3]float64
	if err := json.Unmarshal(data, &v); err != nil {
		return err
	}

	p.Time = v[0]
	p.Seqnum = v[1]
	p.Value = v[2]

	return nil
}

func (p *PointsItemNoNum) UnmarshalJSON(data []byte) error {
	var v [3]float64
	if err := json.Unmarshal(data, &v); err != nil {
		return err
	}

	p.Time = v[0]
	p.Value = v[1]

	return nil
}

func ParseBody(body []byte, metric string) ([]Result, error) {
	var result Result
	var msg MessageNoNum
	var results = make([]Result, 0)

	result.Dps = make(map[string]float64)
	len := len(string(body)) - 1
	body1 := string(string(body)[1:len])

	//if true == strings.Contains(metric, "cpu-idle") {
	if err := json.Unmarshal([]byte(body1), &msg); err != nil {
		return results, err
	}

	result.Metric = msg.Name

	for _, item := range msg.Points {
		result.Dps[fmt.Sprintf("%f", item.Time)] = item.Value
	}
	/*} else {
		var msg MessageWithNum
		if err := json.Unmarshal([]byte(body1), &msg); err != nil {
			return results, err
		}

		result.Metric = msg.Name

		for _, item := range msg.Points {
			result.Dps[fmt.Sprintf("%f", item.Time)] = item.Value
		}
	}*/

	results = append(results, result)

	return results, nil
}

func NewQueryParams(host_id string, start int, end int, rawTags string, aggregator string, metric string, dsname string, function string, order string) string {
/*	if true == strings.Contains(metric, "cpu-idle") {
		return fmt.Sprintf("select derivative(value) from \"%s/%s\" group by time(20s) where time > now() - %dm and time < now() - %dm", host_id, metric, start, end)
	} else if true == strings.Contains(metric, "if_octets-eth0") {
		return fmt.Sprintf("select derivative(value) from \"%s/%s\" group by time(20s) where time > now() - %dm and time < now() - %dm and dsname = 'tx'", host_id, metric, start, end)
	} else if true == strings.Contains(metric, "if_errors-eth0") {
		return fmt.Sprintf("select derivative(value) from \"%s/%s\" group by time(20s) where time > now() - %dm and time < now() - %dm and dsname = 'tx'", host_id, metric, start, end)
	} else if true == strings.Contains(metric, "df") {
		return fmt.Sprintf("select mean(value) from \"%s/%s\" group by time(10s) where time > now() - %dm and time < now() - %dm and dsname='free'", host_id, metric, start, end)
	} else if true == strings.Contains(metric, "load") {
		return fmt.Sprintf("select mean(value) from \"%s/%s\" group by time(10s) where time > now() - %dm and time < now() - %dm and dsname='midterm'", host_id, metric, start, end)
	} else {
		return fmt.Sprintf("select mean(value) from \"%s/%s\" group by time(10s) where time > now() - %dm and time < now() - %dm", host_id, metric, start, end)
	}
*/
	tmp_dsname := "value"
        if dsname != "" {
            tmp_dsname = dsname
        }
        tmp_function :=  "mean"
        if function != "" {
            tmp_function = function
        }
        tmp_order := "asc"
        if order != "" {
            tmp_order = order
        }
        lg.Info(fmt.Sprintf("select %s(value) from \"%s/%s\" group by time(20s) where dsname = \"%s\" and time > now() - %dm and time < now() - %dm order %s",tmp_function,host_id,metric,tmp_dsname,start,end,tmp_order))
	return fmt.Sprintf("select %s(value) from \"%s/%s\" group by time(20s) where dsname = '%s' and time > now() - %dm and time < now() - %dm order %s",tmp_function,host_id,metric,tmp_dsname,start,end,tmp_order);
}

func (c *Client) Query(q string, metric string) ([]Result, error) {
	client := &http.Client{}
	req, err := http.NewRequest("GET", c.url.String(), nil)
	if err != nil {
		return nil, err
	}

	lg.Debug(q)

	params := req.URL.Query()
	params.Set("q", q)
	req.URL.RawQuery = params.Encode()

	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}

	defer resp.Body.Close()

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	lg.Debug(string(body))

	if resp.StatusCode == 200 {
		var results []Result
		var p_err error

		if 0 == strings.Compare(string(body), "[]") {
			return nil, nil
		}

		results, p_err = ParseBody(body, metric)
		if p_err != nil {
			return nil, p_err
		}

		//fmt.Print("-------------%v--------------\n", results[0])

		return results, nil
	} else {
		return nil, errors.New(fmt.Sprintf("Http resp err:%d", resp.StatusCode))
	}

	return nil, nil
}

/*
func (c *Client) tsdb_Query(q *QueryParams) ([]Result, error) {
	data, err := json.Marshal(q)
	if err != nil {
		return nil, err
	}

	u := c.url
	u.Path = "api/query"

	req, err := http.NewRequest("POST", u.String(), bytes.NewReader(data))
	if err != nil {
		return nil, err
	}
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	if resp.StatusCode == 200 {
		var results []Result
		if err := json.Unmarshal(body, &results); err != nil {
			return nil, err
		}
		return results, nil
	} else {
		var err_resp ErrorResp
		if err := json.Unmarshal(body, &err_resp); err != nil {
			return nil, errors.New(string(data))
		}
		return nil, errors.New((&err_resp).String())
	}
}
*/
