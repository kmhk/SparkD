//
//  ReportVC.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class ReportVC: UIViewController {
    
    var reports = [Any]()
    
    @IBOutlet weak var tableView: UITableView!
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        tableView.dataSource = self
        tableView.delegate = self
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let titleView = navigationController?.navTitleWithImageAndText(titleText: "REPORT")
        tabBarController?.navigationItem.titleView = titleView
        
        let exportBtn = UIBarButtonItem(title: "Export", style: .plain, target: self, action: #selector(exportBtnTapped(_:)))
        tabBarController?.navigationItem.rightBarButtonItem = exportBtn
        
        let delBtn = UIBarButtonItem(title: "Erase", style: .done, target: self, action: #selector(deleteBtnTapped(_:)))
        tabBarController?.navigationItem.leftBarButtonItem = delBtn
        
        // get reports
        if let tmp = UserDefaults.standard.value(forKey: "ReportArray") {
            reports = tmp as! [Any]
        }
        
        for i in 0..<reports.count {
            var dict = (reports[i] as! [String: Any])
            dict["selected"] = (i == 0 ? true : false)
            reports[i] = dict
        }
        
        tableView.reloadData()
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

    @objc func exportBtnTapped(_ sender: Any) {
        
    }
    
    
    @objc func deleteBtnTapped(_ sender: Any) {
        var i = 0
        while i < reports.count {
            let dict = reports[i] as! [String: Any]
            if (dict["selected"] as! Bool) == true {
                reports.remove(at: i)
            } else {
                i = i + 1
            }
        }
        
        UserDefaults.standard.setValue(reports, forKey: "ReportArray")
        UserDefaults.standard.synchronize()
        
        tableView.reloadData()
    }
}

// MARK: -
extension ReportVC: UITableViewDataSource, UITableViewDelegate {
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return reports.count
    }
    
    func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        return "Total report (\(reports.count))"
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "ReportTableViewCell", for: indexPath) as! ReportTableViewCell
        
        let report = reports[indexPath.row] as! [String: Any]
        
        cell.lblResult.text = String(format: "%.f", report["average"] as! Double)
        cell.lblName.text = report["title"] as? String
        cell.lblState.text = "Good"
        
        let formatter = DateFormatter()
        formatter.dateFormat = "dd MMM, yyyy"
        let date = Date(timeIntervalSinceNow: report["timestamp"] as! TimeInterval)
        cell.lblDate.text = formatter.string(from: date)
        
        if (report["selected"] as! Bool) == true {
            cell.accessoryType = .checkmark
        } else {
            cell.accessoryType = .none
        }
        
        return cell
    }
    
    func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
        return 90
    }
    
    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)
        
        var report = reports[indexPath.row] as! [String: Any]
        if (report["selected"] as! Bool) == true {
            report["selected"] = false
        } else {
            report["selected"] = true
        }
        reports[indexPath.row] = report
        
        tableView.reloadData()
    }
}
