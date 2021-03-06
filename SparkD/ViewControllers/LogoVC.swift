//
//  LogoVC.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit
import TOPasscodeViewController
import SwiftyGif

class AnimationView: UIView {
    
    var gifView: UIImageView = {
        guard let gifImage = try? UIImage(gifName: "sparkgif.gif") else { return UIImageView() }
        
        return UIImageView(gifImage: gifImage, loopCount: 1)
    }()
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        
        self.backgroundColor = .white
        
        gifView.frame = CGRect(x: 0, y: 0, width: 150, height: 150)
        gifView.center = self.center
        print("\(gifView.frame)")
        addSubview(gifView)
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }
}

class LogoVC: UIViewController {
    
    var mustShowMain = false

    @IBOutlet weak var btnStartTest: UIButton!
    @IBOutlet weak var btnPin: UIButton!
    
    @IBOutlet weak var lblNote: UILabel!
    
    let logoView = AnimationView(frame: UIScreen.main.bounds)
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        view.addSubview(logoView)
        logoView.gifView.delegate = self
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
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
        if logoView.isHidden == false {
            logoView.gifView.startAnimatingGif()
        }
        
        if mustShowMain == true {
            self.performSegue(withIdentifier: "segueMain", sender: nil)
        }
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
        let isActivePin = UserDefaults.standard.value(forKey: "IsActivePin")
        if isActivePin == nil || (isActivePin! as! Bool) == false {
            self.performSegue(withIdentifier: "segueMain", sender: nil)
            return
        }
        
        let vc = TOPasscodeViewController(style: .opaqueLight, passcodeType: .fourDigits)
        vc.addFogotButton()
        vc.delegate = self
        present(vc, animated: true, completion: nil)
    }
    
}


// MARK: -
extension LogoVC: SwiftyGifDelegate {
    func gifDidStop(sender: UIImageView) {
        mustShowMain = false
        
        if UserDefaults.standard.value(forKey: "start running") != nil {
            self.logoView.isHidden = true
            self.performSegue(withIdentifier: "segueMain", sender: nil)
            return
        }
        
        UserDefaults.standard.setValue(true, forKey: "start running")
        UserDefaults.standard.synchronize()
        
        let storyboard = UIStoryboard(name: "Main", bundle: nil)
        let vc = storyboard.instantiateViewController(withIdentifier: "IntroVC") as! IntroVC
        vc.modalPresentationStyle = .fullScreen
        self.present(vc, animated: false) {
            self.logoView.isHidden = true
            
            let isActivePin = UserDefaults.standard.value(forKey: "IsActivePin")
            if isActivePin == nil || (isActivePin! as! Bool) == false {
                self.mustShowMain = true
            }
        }
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

// MARK: -
extension TOPasscodeViewController {
    func addFogotButton() {
        let btn = UIButton(frame: CGRect(x: 20, y: view.frame.height - 60, width: 150, height: 35))
        btn.setTitle("Forgot Password", for: .normal)
        btn.setTitleColor(.systemBlue, for: .normal)
        btn.addTarget(self, action: #selector(btnForgotPwdTapped(_:)), for: .touchUpInside)
        view.addSubview(btn)
    }
    
    @objc func btnForgotPwdTapped(_ sender: Any) {
        
        UserDefaults.standard.set("1234", forKey: "Pin")
        UserDefaults.standard.synchronize()
        
        let alert = UIAlertController(title: nil, message: "Your pin changed to 1234", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .cancel, handler: nil))
        present(alert, animated: true, completion: nil)
    }
}
