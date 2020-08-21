package brainflow;

import com.google.gson.Gson;

/**
 * describe model parameters
 */
public class BrainFlowModelParams
{
    public int metric;
    public int classifier;
    public String file;
    public String other_info;

    public BrainFlowModelParams (int metric, int classifier)
    {
        this.metric = metric;
        this.classifier = classifier;
        this.file = "";
        this.other_info = "";
    }

    public int get_metric ()
    {
        return metric;
    }

    public void set_metric (int metric)
    {
        this.metric = metric;
    }

    public int get_classifier ()
    {
        return classifier;
    }

    public void set_classifier (int classifier)
    {
        this.classifier = classifier;
    }

    public String get_file ()
    {
        return file;
    }

    public void set_file (String file)
    {
        this.file = file;
    }

    public String get_other_info ()
    {
        return other_info;
    }

    public void set_other_info (String other_info)
    {
        this.other_info = other_info;
    }

    public String to_json ()
    {
        return new Gson ().toJson (this);
    }

}
