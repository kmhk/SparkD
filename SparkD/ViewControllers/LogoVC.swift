//
//  LogoVC.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit
import TOPasscodeViewController

class LogoVC: UIViewController {

    @IBOutlet weak var btnStartTest: UIButton!
    @IBOutlet weak var btnPin: UIButton!
    
    @IBOutlet weak var lblNote: UILabel!
    
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
    
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        btnStartTest.roundGradientButton(title: "Start Test")
        btnPin.roundSolidButton(title: "Use your Pin")
        
        let str1 = NSMutableAttributedString(string: "By using the app, you accept our ",
                                             attributes: [NSAttributedString.Key.font: UIFont.systemFont(ofSize: 12.0),
                                                          NSAttributedString.Key.foregroundColor: UIColor.black])
        let str2 = NSMutableAttributedString(string: "general T&C",
                                             attributes: [NSAttributedString.Key.font: UIFont.systemFont(ofSize: 12.0),
                                                          NSAttributedString.Key.foregroundColor: UIColor.purple])
        str1.append(str2)
        lblNote.attributedText = str1
    }
    

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

    
    @IBAction func btnStartTest(_ sender: Any) {
        self.performSegue(withIdentifier: "segueTest", sender: nil)
    }
    
    
    @IBAction func btnPin(_ sender: Any) {
        if let isActivePin = UserDefaults.standard.value(forKey: "IsActivePin") {
            if (isActivePin as! Bool) == false {
                self.performSegue(withIdentifier: "segueMain", sender: nil)
                return
            }
        }
        
        let vc = TOPasscodeViewController(style: .opaqueLight, passcodeType: .fourDigits)
        vc.delegate = self
        present(vc, animated: true, completion: nil)
    }
    
}


// MARK: -

extension LogoVC: TOPasscodeViewControllerDelegate {
    
    func didInputCorrectPasscode(in passcodeViewController: TOPasscodeViewController) {
        dismiss(animated: true) {
            self.performSegue(withIdentifier: "segueMain", sender: nil)
        }
    }
    
    func didTapCancel(in passcodeViewController: TOPasscodeViewController) {
        dismiss(animated: true, completion: nil)
    }
    
    
    func passcodeViewController(_ passcodeViewController: TOPasscodeViewController, isCorrectCode code: String) -> Bool {
        if let pin = UserDefaults.standard.value(forKey: "Pin") {
            if (pin as! String) == code {
                return true
            }
            
            return false
        }
        
        if code == "1234" {
            return true
        }
        
        return false
    }
    
}
